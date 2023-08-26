#ifndef AT_PARSER_H
#define AT_PARSER_H

#include <Arduino.h>
#include "./AtCommandHandler.hpp"

typedef class AtParser AtParser;

typedef enum {
  PREFIX_A,
  PREFIX_T,
  COMMAND,
  READ,
  TEST,

  TEST_OR_PARAMETER_START,
  PARAMETER_START,
  PARAMETER_STRING,
  PARAMETER_NUMBER,
  PARAMETER_DELIMITER_OR_END,
  PARAMETER_TERMINATOR,
  PASSTHROUGH,
} AT_PARSER_STATE;

typedef int16_t AT_COMMAND_RETURN_TYPE;

class AtParser {
public:
  AtParser() {}
  void begin(Stream *serial, AtCommandHandler **commands, uint32_t size, char *buffer) {
    this->buffer = buffer;
    this->atCommands = commands;
    this->numberOfCommands = (uint16_t)(size / sizeof(AtCommandHandler*));
    this->serial = serial;
  }

  void parse(char current) {
    switch (this->state) {
    case PREFIX_A:
      if (current == 'A') {
        this->state = PREFIX_T;
      }
      break;

    case PREFIX_T:
      if (current == 'T') {
        this->state = COMMAND;
      } else {
        this->state = PREFIX_A;
      }
      break;

    case COMMAND:
      (this->buffer)[bufferPosition] = 0;
      if (current == '?') {
        bufferPosition++;
        this->state = READ;
        return;
      } else if (current == '=') {
        this->state = TEST_OR_PARAMETER_START;
        bufferPosition++;
        return;
      } else if (current == '\r') {
        this->run();
        return;
      } else {
        buffer[bufferPosition] = current;
      }
      bufferPosition++;
      break;

    case TEST_OR_PARAMETER_START:
      if (current == '?') {
        this->state = TEST;
        return;
      }
      this->parameterStartPosition = bufferPosition;
      this->state = PARAMETER_START;
    case PARAMETER_START:
      if (current == '"') {
        this->state = PARAMETER_STRING;
        return;
      }
      this->state = PARAMETER_NUMBER;
    case PARAMETER_NUMBER:
      if (this->handleParameterDelimiterOrEnd(current)) {
        return;
      }
      this->writeToBuffer(current);
      break;

    case PARAMETER_STRING:
      if (current == '"') {
        this->state = PARAMETER_DELIMITER_OR_END;
        return;
      }
      this->writeToBuffer(current);
      break;

    case PARAMETER_DELIMITER_OR_END:
      if (!this->handleParameterDelimiterOrEnd(current)) {
        this->error();
        return;
      }
      break;

    case PARAMETER_TERMINATOR:
      if (current == '\n') {
        this->write();
      } else {
        this->error();
      }
      break;

    case READ:
      if (current == '\r') {
        this->read();
      } else {
        this->error();
      }
      break;

    case TEST:
      if (current == '\r') {
        this->test();
      } else {
        this->error();
      }
      break;

    case PASSTHROUGH:
      writeToBuffer(current);
      remainingPassthroughChars--;
      if (remainingPassthroughChars <= 0) {
        writeToBuffer(0);
        handlePassthroughEnd();
        return;
      }
      break;
    }
  }

  uint8_t getNumParameters() const { return this->numParameters; }

  const char *getNextParameter() {
    if (this->numParameters == 0) {
      return nullptr;
    }
    if (this->currentParameterToRead == 0) {
      this->parameterReadPosition = this->parameterStartPosition;
    }
    const char *param = &buffer[this->parameterReadPosition];
    this->currentParameterToRead++;
    if (this->currentParameterToRead >= this->numParameters) {
      this->currentParameterToRead = 0;
    } else {
      this->parameterReadPosition += strlen(param) + 1;
    }
    return param;
  }

  const char *getBuffer() { return buffer; }

  Stream *serial;

private:
  char *buffer;
  uint16_t bufferPosition = 0;
  AT_PARSER_STATE state = PREFIX_A;
  uint16_t numberOfCommands;
  AtCommandHandler **atCommands;

  AtCommandHandler *currentCommand = nullptr;
  uint8_t numParameters = 0;
  uint8_t parameterStartPosition = 0;
  uint8_t parameterReadPosition = 0;
  uint8_t currentParameterToRead = 0;

  uint16_t remainingPassthroughChars = 0;

  boolean handleParameterDelimiterOrEnd(char current) {
    if (current == ',') {
      this->writeToBuffer(0);
      this->state = PARAMETER_START;
      numParameters++;
      return true;
    }
    if (current == '\r') {
      this->writeToBuffer(0);
      numParameters++;
      state = PARAMETER_TERMINATOR;
      return true;
    }
    return false;
  }

  void writeToBuffer(char current) {
    (this->buffer)[bufferPosition] = current;
    this->bufferPosition++;
  }

  void setCommand() {
    for (uint8_t i = 0; i < this->numberOfCommands; i++) {
      const char* name = ((this->atCommands)[i])->getName();
      if (strncmp(buffer, name, 100) == 0) {
        currentCommand = atCommands[i];
        return;
      }
    }
  }

  void handleCommandResult(AT_COMMAND_RETURN_TYPE result) {
    if (result == 0) {
      serial->print("OK\r\n");
    }
    if (result < 0) {
      serial->print("ERROR\r\n");
    }
    if (result > 0) {
      serial->print(">\r\n");
      this->state = PASSTHROUGH;
      this->remainingPassthroughChars = result;
      this->bufferPosition = 0;
      this->numParameters = 0;
      this->parameterStartPosition = 0;
      return;
    }
    reset();
    return;
  }

  void handlePassthroughEnd() {
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->passthrough(this));
      return;
    }
    serial->print("ERROR\r\n");
  }

  void run() {
    setCommand();
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->run(this));
      return;
    }
    serial->print("ERROR\r\n");
    reset();
  }

  void read() {
    setCommand();
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->read(this));
      return;
    }
    serial->print("ERROR\r\n");
    reset();
  }

  void test() {
    setCommand();
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->test(this));
      return;
    }
    serial->print("ERROR\r\n");
    reset();
  }

  void write() {
    setCommand();
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->write(this));
      return;
    }
    serial->print("ERROR\r\n");
    reset();
  }

  void error() { reset(); }

  void reset() {
    this->bufferPosition = 0;
    this->state = PREFIX_A;
    this->currentCommand = nullptr;
    this->numParameters = 0;
    this->parameterStartPosition = 0;
    this->remainingPassthroughChars = 0;
  }
};

#endif
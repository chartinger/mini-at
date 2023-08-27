#ifndef MINI_AT_PARSER_H
#define MINI_AT_PARSER_H

#include "./MiniAtParserCommandHandler.hpp"
#include <Arduino.h>

typedef class MiniAtParser MiniAtParser;

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

class MiniAtParser {
public:
  MiniAtParser() {}
  void begin(Stream *serial, MiniAtParserCommandHandler **commands, uint32_t size, char *buffer, uint32_t bufferSize) {
    this->buffer = buffer;
    this->atCommands = commands;
    this->numberOfCommands = (uint16_t)(size / sizeof(MiniAtParserCommandHandler *));
    this->serial = serial;
    this->bufferSize = bufferSize;
    this->reset();
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
        if (!this->advanceBufferPosition()) {
          error();
        }
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
      this->state = PARAMETER_START;
    case PARAMETER_START:
      this->num_args = this->num_args + 1;
      this->args[this->num_args - 1] = &this->buffer[bufferPosition];
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
  Stream *serial;

private:
  char *buffer;
  uint16_t bufferSize = 0;
  uint16_t bufferPosition = 0;
  AT_PARSER_STATE state = PREFIX_A;
  uint16_t numberOfCommands;
  MiniAtParserCommandHandler **atCommands;

  MiniAtParserCommandHandler *currentCommand = nullptr;

  uint16_t numPassthroughChars = 0;
  uint16_t remainingPassthroughChars = 0;

  uint8_t max_args = 10;
  uint8_t num_args = 0;
  char *args[10];

  boolean handleParameterDelimiterOrEnd(char current) {
    if (current == ',') {
      this->writeToBuffer(0);
      this->state = PARAMETER_START;
      return true;
    }
    if (current == '\r') {
      this->writeToBuffer(0);
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
      const char *name = ((this->atCommands)[i])->getName();
      if (strncmp(buffer, name, 100) == 0) {
        currentCommand = atCommands[i];
        return;
      }
    }
  }

  void handleCommandResult(AT_COMMAND_RETURN_TYPE result) {
    if (result == 0) {
      serial->print("\r\nOK\r\n");
    }
    if (result < 0) {
      serial->print("\r\nERROR\r\n");
    }
    if (result > 0) {
      if (result >= this->bufferSize - 1) {
        serial->print("\r\nERROR\r\n");
        this->error();
        return;
      }
      serial->print(">\r\n");
      this->state = PASSTHROUGH;
      this->remainingPassthroughChars = result;
      this->numPassthroughChars = result;
      this->bufferPosition = 0;
      this->num_args = 0;
      return;
    }
    reset();
    return;
  }

  void handlePassthroughEnd() {
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->passthrough(this->serial, this->buffer, this->numPassthroughChars));
      return;
    }
    serial->print("\r\nERROR\r\n");
    reset();
  }

  void run() {
    setCommand();
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->run(this->serial));
      return;
    }
    serial->print("\r\nERROR\r\n");
    reset();
  }

  void read() {
    setCommand();
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->read(this->serial));
      return;
    }
    serial->print("\r\nERROR\r\n");
    reset();
  }

  void test() {
    setCommand();
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->test(this->serial));
      return;
    }
    serial->print("\r\nERROR\r\n");
    reset();
  }

  void write() {
    setCommand();
    if (this->currentCommand != nullptr) {
      handleCommandResult(this->currentCommand->write(this->serial, (char **)(this->args), this->num_args));
      return;
    }
    serial->print("\r\nERROR\r\n");
    reset();
  }

  void error() { reset(); }

  void reset() {
    this->bufferPosition = 0;
    this->state = PREFIX_A;
    this->currentCommand = nullptr;
    this->remainingPassthroughChars = 0;
    this->numPassthroughChars = 0;
    this->num_args = 0;
  }

  boolean advanceBufferPosition() {
    if (this->bufferPosition >= this->bufferSize - 2) {
      return false;
    }
    return true;
  }
};

#endif
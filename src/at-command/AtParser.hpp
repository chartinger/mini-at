#ifndef AT_PARSER_H
#define AT_PARSER_H

#include <Arduino.h>

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
  PARAMETER_DELIMITER_OR_END
} AT_PARSER_STATE;

typedef int16_t AT_COMMAND_RETURN_TYPE;

/**
 * @brief at_command_t struct
 *  borrowed verbatim from esp-at and is used to define an AT command
 */
typedef struct {
  const char *at_cmdName;                                 // command name
  AT_COMMAND_RETURN_TYPE (*at_runCmd)(AtParser *);        // RUN command function pointer
  AT_COMMAND_RETURN_TYPE (*at_testCmd)(AtParser *);       // TEST command function pointer
  AT_COMMAND_RETURN_TYPE (*at_readCmd)(AtParser *);       // READ command function pointer
  AT_COMMAND_RETURN_TYPE (*at_writeCmd)(AtParser *);      // WRITE command function pointer
  AT_COMMAND_RETURN_TYPE (*at_passtroughCmd)(AtParser *); // PASSTHROUGH command
} at_command_t;

class AtParser {
public:
  AtParser() {}
  void begin(Stream *serial, const at_command_t *commands, uint32_t size, char *buffer) {
    this->buffer = buffer;
    (this->buffer)[0] = 'A';
    this->atCommands = commands;
    this->numberOfCommands = (uint16_t)(size / sizeof(at_command_t));
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
        this->state = READ;
        return;
      } else if (current == '=') {
        this->state = TEST_OR_PARAMETER_START;
        return;
      } else if (current == '\r') {
        this->run();
        this->reset();
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
    }
  }

  uint8_t getNumParameters() { return this->numParameters; }

  char *getNextParameter() {
    if (this->numParameters == 0) {
      return nullptr;
    }
    if (this->currentParameterToRead == 0) {
      this->parameterReadPosition = this->parameterStartPosition;
    }
    // char param = this->buffer[this->parameterReadPosition];
    // this->currentParameterToRead++;
    // if (this->currentParameterToRead >= this->numParameters) {
    //   this->currentParameterToRead = 0;
    // } else {
    //   // this->parameterReadPosition += strlen(param) + 1;
    // }
  }

  AT_PARSER_STATE getState() { return this->state; }

  uint16_t getDebug() { return debug; }

private:
  char *buffer;
  uint16_t bufferPosition = 0;
  AT_PARSER_STATE state = PREFIX_A;
  uint16_t debug = 0;
  uint16_t numberOfCommands;
  const at_command_t *atCommands;
  Stream *serial;

  const at_command_t *currentCommand = nullptr;
  uint8_t numParameters = 0;
  uint8_t parameterStartPosition = 0;
  uint8_t parameterReadPosition = 0;
  uint8_t currentParameterToRead = 0;

  boolean handleParameterDelimiterOrEnd(char current) {
    if (current == ',') {
      this->writeToBuffer(0);
      this->state = PARAMETER_START;
      return true;
    }
    if (current == '\r') {
      this->writeToBuffer(0);
      this->write();
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
      if (strncmp(buffer, (this->atCommands)[i].at_cmdName, 100) == 0) {
        currentCommand = (&atCommands[i]);
        return;
      }
    }
  }

  void run() {
    setCommand();
    if (this->currentCommand != nullptr && this->currentCommand->at_runCmd != nullptr) {
      (this->currentCommand->at_runCmd)(this);
    }
  }

  void read() {
    setCommand();
    if (this->currentCommand != nullptr && this->currentCommand->at_readCmd != nullptr) {
      (this->currentCommand->at_readCmd)(this);
    }
  }

  void test() {
    setCommand();
    if (this->currentCommand != nullptr && this->currentCommand->at_testCmd != nullptr) {
      (this->currentCommand->at_testCmd)(this);
    }
  }

  void write() {
    setCommand();
    if (this->currentCommand != nullptr && this->currentCommand->at_writeCmd != nullptr) {
      (this->currentCommand->at_writeCmd)(this);
    }
  }

  void error() { reset(); }

  void reset() {
    this->bufferPosition = 1;
    this->state = PREFIX_A;
    this->currentCommand = nullptr;
    this->numParameters = 0;
    this->parameterStartPosition = 0;
  }
};

#endif
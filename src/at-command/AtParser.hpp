#ifndef AT_PARSER_H
#define AT_PARSER_H

#include <Arduino.h>

typedef class AtParser AtParser;

typedef enum
{
    PREFIX_A,
    PREFIX_T,
    COMMAND,
    WRITE,
    WRITE_START,
    READ,
    TEST,
    TEST_OR_VARIABLE,
} AT_PARSER_STATE;


typedef int16_t AT_COMMAND_RETURN_TYPE;

/**
 * @brief at_command_t struct
 *  borrowed verbatim from esp-at and is used to define an AT command 
*/
typedef struct
{
    char *at_cmdName;                  // command name
    AT_COMMAND_RETURN_TYPE (*at_runCmd)(AtParser *);   // RUN command function pointer
    AT_COMMAND_RETURN_TYPE (*at_testCmd)(AtParser *);  // TEST command function pointer
    AT_COMMAND_RETURN_TYPE (*at_readCmd)(AtParser *);  // READ command function pointer
    AT_COMMAND_RETURN_TYPE (*at_writeCmd)(AtParser *); // WRITE command function pointer
    AT_COMMAND_RETURN_TYPE (*at_passtroughCmd)(AtParser *); // PASSTHROUGH command
} at_command_t;


class AtParser {
  public:
    void parse(char current) {
      switch(this->state) {
        case PREFIX_A:
          if(current == 'A') {
            this->state = PREFIX_T;
          }
          break;
        case PREFIX_T:
          if(current == 'T') {
            this->state = COMMAND;
          } else {
            this->state = PREFIX_A;
          }
          break;
        case COMMAND:
          *this->buffer[bufferPosition] = 0;
          if(current == '?') {
            this->state = READ;
            return;
          } else 
          if(current == '=') {
            this->state = WRITE_START;
            return;
          } else 
          if(current == '\r') {
            this->run();
            return;
          } else {
            *this->buffer[bufferPosition] = current;
          }
          bufferPosition++;
          break;
        case WRITE_START:
          if(current == '?') {
            this->state = TEST;
            return;
          } else 
          if(current == '\r') {
            this->error();
            return;
          } else if(current == ',') {
            *this->buffer[bufferPosition] = 0;
            this->numParameters++;
          } else {
            *this->buffer[bufferPosition] = current;
          }
          this->parameterStartPosition = bufferPosition;
          this->bufferPosition++;
          this->state = WRITE;
          break;
        case WRITE:
          if(current == '\r') {
            *this->buffer[bufferPosition] = 0;
            this->numParameters++;
            this->write();
            return;
          } else if(current == ',') {
            *this->buffer[bufferPosition] = 0;
            this->numParameters++;
          } else {
            *this->buffer[bufferPosition] = current;
          }
          this->bufferPosition++;
          break;
        case READ:
          if(current == '\r') {
            this->read();
          } else {
            this->error();
          }
          break;
        case TEST:
          if(current == '\r') {
            this->test();
          } else {
            this->error();
          }
          break;
      }
    }

  private:
    char* buffer[256];
    uint8_t bufferPosition = 0;
    AT_PARSER_STATE state = PREFIX_A;

    uint16_t numberOfCommands;
    const at_command_t *atCommands;

    at_command_t *currentCommand = nullptr;
    uint8_t numParameters = 0;
    uint8_t parameterStartPosition = 0;
    uint8_t parameterReadPosition = 0;
    uint8_t currentParameterToRead = 0;

    void setCommand() {
      for (uint8_t i = 0; i < this->numberOfCommands; i++)
      {
        if (strcmp((const char*)buffer, atCommands[i].at_cmdName) == 0)
        {
          *this->currentCommand = this->atCommands[i];
        }
      }
    }

    char* getNextParameter() {
      if(this->numParameters == 0) {
        return nullptr;
      }
      if(this->currentParameterToRead == 0) {
        this->parameterReadPosition = this->parameterStartPosition;
      }
      char* param = this->buffer[this->parameterReadPosition];
      this->currentParameterToRead++;
      if(this->currentParameterToRead >= this->numParameters) {
        this->currentParameterToRead = 0;
      } else {
        this->parameterReadPosition += strlen(param) + 1;
      }
    }

    void run() {
      setCommand();
      if(this->currentCommand != nullptr && this->currentCommand->at_runCmd != nullptr) {
        (this->currentCommand->at_runCmd)(this);
      }
    }

    void read() {
      setCommand();
      if(this->currentCommand != nullptr && this->currentCommand->at_readCmd != nullptr) {
        (this->currentCommand->at_readCmd)(this);
      }
    }

    void test() {
      setCommand();
      if(this->currentCommand != nullptr && this->currentCommand->at_testCmd != nullptr) {
        (this->currentCommand->at_testCmd)(this);
      }
    }

    void write() {
      setCommand();
      if(this->currentCommand != nullptr && this->currentCommand->at_writeCmd != nullptr) {
        (this->currentCommand->at_writeCmd)(this);
      }
    }

    void error() {
      reset();
    }

    void reset() {
      this->bufferPosition = 0;
      this->state = PREFIX_A;
      this->currentCommand = nullptr;
      this->numParameters = 0;
      this->parameterStartPosition = 0;
    }
};

#endif
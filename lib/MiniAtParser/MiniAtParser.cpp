#include "./MiniAtParser.hpp"

MiniAtParser::MiniAtParser(){};
MiniAtParser::~MiniAtParser(){};

void MiniAtParser::begin(Stream* serial, MiniAtParserCommandHandler** commands, uint32_t size, char* buffer, uint32_t bufferSize) {
  this->buffer = buffer;
  this->atCommands = commands;
  this->numberOfCommands = (uint16_t)(size / sizeof(MiniAtParserCommandHandler*));
  this->serial = serial;
  this->bufferSize = bufferSize;
  this->reset();
};

void MiniAtParser::parse(char current) {
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
        lockInCommand();
        this->state = READ;
        return;
      } else if (current == '=') {
        lockInCommand();
        this->state = TEST_OR_PARAMETER_START;
        return;
      } else if (current == '\r') {
        lockInCommand();
        this->run();
        return;
      } else {
        if (!this->writeToBuffer(current)) {
          error();
          return;
        }
      }
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
      if (!this->writeToBuffer(current)) {
        error();
        return;
      }
      break;

    case PARAMETER_STRING:
      if (current == '"') {
        this->state = PARAMETER_DELIMITER_OR_END;
        return;
      }
      if (!this->writeToBuffer(current)) {
        error();
        return;
      }
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

boolean MiniAtParser::handleParameterDelimiterOrEnd(char current) {
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

boolean MiniAtParser::writeToBuffer(char current) {
  if (this->bufferPosition >= this->bufferSize - 1) {
    return false;
  }
  (this->buffer)[bufferPosition] = current;
  this->bufferPosition++;
  return true;
}

void MiniAtParser::lockInCommand() {
  this->bufferPosition = 0;
  for (uint8_t i = 0; i < this->numberOfCommands; i++) {
    const char* name = ((this->atCommands)[i])->getName();
    if (strncmp(buffer, name, this->bufferSize) == 0) {
      currentCommand = atCommands[i];
      return;
    }
  }
}

void MiniAtParser::handleCommandResult(AT_COMMAND_RETURN_TYPE result) {
  if (result == 0) {
    serial->print("\r\nOK\r\n");
  }
  if (result < 0) {
    serial->print("\r\nERROR\r\n");
  }
  if (result > 0) {
    // Check if buffer is big enough for all passthrough data but
    // reserve last byte for null termination (just in case)
    if (result > this->bufferSize - 1) {
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

void MiniAtParser::handlePassthroughEnd() {
  if (this->currentCommand != nullptr) {
    handleCommandResult(this->currentCommand->passthrough(this->serial, this->buffer, this->numPassthroughChars));
    return;
  }
  this->error();
}

void MiniAtParser::run() {
  if (this->currentCommand != nullptr) {
    handleCommandResult(this->currentCommand->run(this->serial));
    return;
  }
  this->error();
}

void MiniAtParser::read() {
  if (this->currentCommand != nullptr) {
    handleCommandResult(this->currentCommand->read(this->serial));
    return;
  }
  this->error();
}

void MiniAtParser::test() {
  if (this->currentCommand != nullptr) {
    handleCommandResult(this->currentCommand->test(this->serial));
    return;
  }
  this->error();
}

void MiniAtParser::write() {
  if (this->currentCommand != nullptr) {
    handleCommandResult(this->currentCommand->write(this->serial, (char**)(this->args), this->num_args));
    return;
  }
  this->error();
}

void MiniAtParser::error() {
  serial->print("\r\nERROR\r\n");
  reset();
}

void MiniAtParser::reset() {
  this->bufferPosition = 0;
  this->state = PREFIX_A;
  this->currentCommand = nullptr;
  this->remainingPassthroughChars = 0;
  this->numPassthroughChars = 0;
  this->num_args = 0;
}

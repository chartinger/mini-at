#pragma once

#ifndef MINI_AT_PARSER_H
#define MINI_AT_PARSER_H

#include <Arduino.h>
#include "./MiniAtParserCommandHandler.hpp"

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
  Stream* serial;

  MiniAtParser();
  ~MiniAtParser();
  MiniAtParser(const MiniAtParser&) = delete;
  MiniAtParser& operator=(const MiniAtParser&) = delete;

  void begin(Stream* serial, MiniAtParserCommandHandler** commands, uint32_t size, char* buffer, uint32_t bufferSize);
  void parse(char current);

 private:
  char* buffer;
  uint16_t bufferSize = 0;
  uint16_t bufferPosition = 0;
  AT_PARSER_STATE state = PREFIX_A;
  uint16_t numberOfCommands;
  MiniAtParserCommandHandler** atCommands;

  MiniAtParserCommandHandler* currentCommand = nullptr;

  uint16_t numPassthroughChars = 0;
  uint16_t remainingPassthroughChars = 0;

  uint8_t max_args = 10;
  uint8_t num_args = 0;
  char* args[10];

  boolean handleParameterDelimiterOrEnd(char current);

  boolean writeToBuffer(char current);

  void lockInCommand();

  void handleCommandResult(AT_COMMAND_RETURN_TYPE result);

  void handlePassthroughEnd();

  void run();

  void read();

  void test();

  void write();

  void error();

  void reset();
};

#endif
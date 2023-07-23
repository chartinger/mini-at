#include "unity.h"
#include <ArduinoFake.h>

#include "./AtTestHelper.hpp"

using namespace fakeit;

#include "../src/at-command/AtParser.hpp"

Mock<CommandHandlerProxy> commandHandlerMock;
Serial_ *MockSerial = ArduinoFakeMock(Serial);

void resetSerial() {
  ArduinoFake(Serial).Reset();
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char *))).AlwaysReturn(1);
}

void setUp(void) {
  ArduinoFakeReset();
  resetSerial();
  commandHandlerMock.Reset();
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  When(Method(commandHandlerMock, test)).AlwaysReturn(0);
  When(Method(commandHandlerMock, read)).AlwaysReturn(0);
  When(Method(commandHandlerMock, write)).AlwaysReturn(0);
  When(Method(commandHandlerMock, passthrough)).AlwaysReturn(0);
}

void tearDown(void) {}

void feedInput(AtParser &parser, const char *data) {
  int length = strnlen(data, 100);
  for (int i = 0; i < length; i++) {
    parser.parse(data[i]);
  }
}

void it_executes_the_run_command(void) {
  at_command_t commands[] = {
      {"+XY", [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().run(); }, nullptr, nullptr, nullptr,
       nullptr},
  };
  AtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer);
  feedInput(parser, "AT+XY\r\n");
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(1));
}

void it_ignores_unknown_commands(void) {
  at_command_t commands[] = {
      {"+XY", [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().run(); }, nullptr, nullptr, nullptr,
       nullptr},
  };
  AtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer);
  feedInput(parser, "AT+WTF\r\n");
  TEST_ASSERT_SERIAL_PRINT("ERROR\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(0));
}

void it_supports_the_write_command(void) {
  AtParser parser;
  char buffer[100];
  at_command_t commands[] = {
      {
          "+XY",
          nullptr,
          nullptr,
          nullptr,
          [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE {
            TEST_ASSERT_EQUAL_INT(5, parser->getNumParameters());
            TEST_ASSERT_EQUAL_STRING("1", parser->getNextParameter());
            TEST_ASSERT_EQUAL_STRING("2", parser->getNextParameter());
            TEST_ASSERT_EQUAL_STRING("3", parser->getNextParameter());
            TEST_ASSERT_EQUAL_STRING("", parser->getNextParameter());
            TEST_ASSERT_EQUAL_STRING("4", parser->getNextParameter());
            return commandHandlerMock.get().write();
          },
          [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().passthrough(); },
      },
  };
  parser.begin(MockSerial, commands, sizeof(commands), buffer);

  feedInput(parser, "AT+XY=1,2,3,,4\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
}

void it_supports_the_write_command_with_empty_parameters(void) {
  AtParser parser;
  char buffer[100];
  at_command_t commands[] = {
      {"+XY", nullptr, nullptr, nullptr,
       [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE {
         TEST_ASSERT_EQUAL_INT(5, parser->getNumParameters());
         TEST_ASSERT_EQUAL_STRING("", parser->getNextParameter());
         TEST_ASSERT_EQUAL_STRING("", parser->getNextParameter());
         TEST_ASSERT_EQUAL_STRING("X", parser->getNextParameter());
         TEST_ASSERT_EQUAL_STRING("", parser->getNextParameter());
         TEST_ASSERT_EQUAL_STRING("", parser->getNextParameter());
         return commandHandlerMock.get().write();
       },
       nullptr},
  };
  parser.begin(MockSerial, commands, sizeof(commands), buffer);
  feedInput(parser, "AT+XY=,,X,,\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
}

void it_supports_the_write_command_with_string_and_number_parameters(void) {
  AtParser parser;
  char buffer[100];
  at_command_t commands[] = {
      {"+XY", nullptr, nullptr, nullptr,
       [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE {
         TEST_ASSERT_EQUAL_INT(4, parser->getNumParameters());
         TEST_ASSERT_EQUAL_STRING("123", parser->getNextParameter());
         TEST_ASSERT_EQUAL_STRING("456", parser->getNextParameter());
         TEST_ASSERT_EQUAL_STRING("Hello", parser->getNextParameter());
         TEST_ASSERT_EQUAL_STRING("World", parser->getNextParameter());
         return commandHandlerMock.get().write();
       },
       nullptr},
  };
  parser.begin(MockSerial, commands, sizeof(commands), buffer);
  feedInput(parser, "AT+XY=123,456,\"Hello\",\"World\"\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
}

void it_supports_the_write_command_with_passthrough(void) {
  AtParser parser;
  char buffer[100];
  at_command_t commands[] = {
      {"+XY", nullptr, nullptr, nullptr,
       [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE {
         TEST_ASSERT_EQUAL_INT(1, parser->getNumParameters());
         TEST_ASSERT_EQUAL_STRING("4", parser->getNextParameter());
         commandHandlerMock.get().write();
         return 10;
       },
       [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE {
         TEST_ASSERT_EQUAL_STRING("Hello You!", parser->getBuffer());
         return commandHandlerMock.get().passthrough();
       }},
  };
  parser.begin(MockSerial, commands, sizeof(commands), buffer);
  TEST_ASSERT_MOCK(feedInput(parser, "AT+XY=4\r\n"));
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(">\r\n");
  resetSerial();
  TEST_ASSERT_MOCK(feedInput(parser, "Hello You!\r\n"));
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), passthrough)).Exactly(1));
}

void it_executes_the_base_commands(void) {
  AtParser parser;
  char buffer[100];
  at_command_t commands[] = {
      {
          "+XY",
          [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().run(); },
          [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().test(); },
          [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().read(); },
          [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().write(); },
          [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().passthrough(); },
      },
  };
  parser.begin(MockSerial, commands, sizeof(commands), buffer);

  // RUN
  feedInput(parser, "AT+XY\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  resetSerial();

  // READ
  TEST_ASSERT_MOCK(feedInput(parser, "AT+XY?\r\n"));
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), read)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  resetSerial();

  // TEST
  feedInput(parser, "AT+XY=?\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), test)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  resetSerial();

  // WRITE
  feedInput(parser, "AT+XY=1,3\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  resetSerial();
}

void it_can_run_multiple_commands(void) {
  at_command_t commands[] = {
      {"+XY", [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().run(); }, nullptr, nullptr, nullptr,
       nullptr},
      {"+AB", [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().run(); }, nullptr, nullptr, nullptr,
       nullptr},
  };

  AtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer);
  feedInput(parser, "AT+XY\r\nAT+AB\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(2));
  TEST_ASSERT_EQUAL_STRING(buffer, "+AB");
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(it_executes_the_run_command);
  RUN_TEST(it_supports_the_write_command);
  RUN_TEST(it_ignores_unknown_commands);
  RUN_TEST(it_supports_the_write_command_with_passthrough);
  RUN_TEST(it_supports_the_write_command_with_empty_parameters);
  RUN_TEST(it_supports_the_write_command_with_string_and_number_parameters);
  RUN_TEST(it_executes_the_base_commands);
  RUN_TEST(it_can_run_multiple_commands);
  return UNITY_END();
}

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) { return runUnityTests(); }

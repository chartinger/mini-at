#include "unity.h"
#include <ArduinoFake.h>

#include "./AtTestHelper.hpp"

using namespace fakeit;

#include "../src/at-command/AtParser.hpp"

Mock<CommandHandlerProxy> commandHandlerMock;
Serial_ *MockSerial = ArduinoFakeMock(Serial);

void setUp(void) {
  ArduinoFakeReset();
  ArduinoFake(Serial).Reset();
  commandHandlerMock.Reset();
}

void tearDown(void) {}

void feedInput(AtParser &parser, const char *data) {
  int length = strnlen(data, 100);
  for (int i = 0; i < length; i++) {
    parser.parse(data[i]);
  }
}

void resetSerial() {
  ArduinoFake(Serial).Reset();
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char *))).AlwaysReturn(1);
}

void it_executes_the_run_command(void) {
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char *))).AlwaysReturn(1);
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

void it_executes_the_base_commands(void) {
  AtParser parser;
  char buffer[100];
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  When(Method(commandHandlerMock, test)).AlwaysReturn(0);
  When(Method(commandHandlerMock, read)).AlwaysReturn(0);
  When(Method(commandHandlerMock, write)).AlwaysReturn(0);
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char *))).AlwaysReturn(1);
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

  // // RUN
  // feedInput(parser, "AT+XY\r\n");
  // TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(1));
  // TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  // resetSerial();

  // // READ
  // TEST_ASSERT_MOCK(feedInput(parser, "AT+XY?\r\n"));
  // TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), read)).Exactly(1));
  // TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  // resetSerial();

  // // TEST
  // feedInput(parser, "AT+XY=?\r\n");
  // TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), test)).Exactly(1));
  // TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  // resetSerial();

  // WRITE
  feedInput(parser, "AT+XY=1,3\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Never());
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), read)).Never());
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), test)).Never());
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  resetSerial();
}

void test_function_should_doBlahAndBlah(void) {
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char *))).AlwaysReturn(1);

  at_command_t commands[] = {
      {"+XY", [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().run(); }, nullptr, nullptr, nullptr,
       nullptr},
      {"+AB", [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE { return commandHandlerMock.get().run(); }, nullptr, nullptr, nullptr,
       nullptr},
  };

  AtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer);
  // TEST_PRINTF("String    %s\n", "My string");
  // TEST_ASSERT_EQUAL_INT(parser.getState(), PREFIX_A);
  // parser.parse('A');
  // TEST_ASSERT_EQUAL_INT(parser.getState(), PREFIX_T);
  // parser.parse('T');
  // // TEST_ASSERT_EQUAL_INT(parser.getState(), COMMAND);
  // parser.parse('+');
  // // TEST_ASSERT_EQUAL_INT(parser.getState(), COMMAND);
  // parser.parse('X');
  // parser.parse('Y');
  // parser.parse('\r');
  // parser.parse('\n');
  feedInput(parser, "AT+XY\r\nAT+AB\r\n");

  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(2));

  TEST_ASSERT_EQUAL_STRING(buffer, "+AB");

  // TEST_ASSERT_EQUAL_INT(parser.getDebug(), 2);
  // TEST_ASSERT_EQUAL_INT(parser.getState(), COMMAND);
  // parser.parse('\r');
  // TEST_ASSERT_EQUAL_INT(parser.getState(), PREFIX_A);
  // parser.parse('G');
  // TEST_ASSERT_EQUAL_INT(parser.getState(), PREFIX_A);
  // parser.parse('M');
  // parser.parse('R');
  // parser.parse('\r');
  // Verify(Method(mockCommand,at_writeCmd)).AtLeastOnce();;
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(it_executes_the_run_command);
  RUN_TEST(it_executes_the_base_commands);
  RUN_TEST(test_function_should_doBlahAndBlah);
  return UNITY_END();
}

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) { return runUnityTests(); }

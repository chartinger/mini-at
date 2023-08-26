#include "unity.h"
#include <ArduinoFake.h>

#include "./AtTestHelper.hpp"

using namespace fakeit;

#include "../src/at-command/AtParser.hpp"

Serial_ *MockSerial = ArduinoFakeMock(Serial);

void resetSerial() {
  ArduinoFake(Serial).Reset();
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char *))).AlwaysReturn(1);
}

void setUp(void) {
  ArduinoFakeReset();
  resetSerial();
}

void tearDown(void) {}

void feedInput(AtParser &parser, const char *data) {
  int length = strnlen(data, 100);
  for (int i = 0; i < length; i++) {
    parser.parse(data[i]);
  }
}

void it_executes_the_run_command(void) {
  Mock<AtCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  AtCommandHandler *commands[1] = {&commandHandlerMock.get()};
  AtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+XY\r\n");
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(1));
}

void it_ignores_unknown_commands(void) {
  Mock<AtCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  AtCommandHandler *commands[1] = {&commandHandlerMock.get()};
  AtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+WTF\r\n");
  TEST_ASSERT_SERIAL_PRINT("ERROR\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(0));
}

void it_supports_the_write_command(void) {
  AtParser parser;
  char buffer[100];
  Mock<AtCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, write)).AlwaysDo([](AtParser *at_parser, char **argv, uint16_t argc) {
    TEST_ASSERT_EQUAL_INT(4, argc);
    TEST_ASSERT_EQUAL_STRING("a", argv[0]);
    TEST_ASSERT_EQUAL_STRING("b", argv[1]);
    TEST_ASSERT_EQUAL_STRING("cc", argv[2]);
    TEST_ASSERT_EQUAL_STRING("dd", argv[3]);
    return 0;
  });
  AtCommandHandler *commands[1] = {&commandHandlerMock.get()};
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+XY=a,b,\"cc\",dd\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
}

void it_supports_the_write_command_with_empty_parameters(void) {
  AtParser parser;
  char buffer[100];
  Mock<AtCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, write)).AlwaysDo([](AtParser *at_parser, char **argv, uint16_t argc) {
    TEST_ASSERT_EQUAL_INT(4, argc);
    TEST_ASSERT_EQUAL_STRING("", argv[0]);
    TEST_ASSERT_EQUAL_STRING("", argv[1]);
    TEST_ASSERT_EQUAL_STRING("X", argv[2]);
    TEST_ASSERT_EQUAL_STRING("", argv[3]);
    return 0;
  });
  AtCommandHandler *commands[1] = {&commandHandlerMock.get()};
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+XY=,,X,\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
}

void it_supports_the_write_command_with_passthrough(void) {
  AtParser parser;
  char buffer[100];
  Mock<AtCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, write)).AlwaysDo([](AtParser *at_parser, char **argv, uint16_t argc) {
    TEST_ASSERT_EQUAL_INT(1, argc);
    return 10;
  });
  When(Method(commandHandlerMock, passthrough)).AlwaysDo([](AtParser *parser, char* data, uint16_t dataLength) {
    TEST_ASSERT_EQUAL_INT(10, dataLength);
    TEST_ASSERT_EQUAL_STRING("Hello You!", data);
    return 0;
  });
  AtCommandHandler *commands[1] = {&commandHandlerMock.get()};
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
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
  Mock<AtCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  When(Method(commandHandlerMock, read)).AlwaysReturn(0);
  When(Method(commandHandlerMock, write)).AlwaysReturn(0);
  When(Method(commandHandlerMock, test)).AlwaysReturn(0);
  AtCommandHandler *commands[1] = {&commandHandlerMock.get()};
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));

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
  feedInput(parser, "AT+XY=\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  resetSerial();
}

void it_can_run_multiple_commands(void) {
  Mock<AtCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  Mock<AtCommandHandler> commandHandlerMock2;
  When(Method(commandHandlerMock2, getName)).AlwaysReturn("+AB");
  When(Method(commandHandlerMock2, run)).AlwaysReturn(0);

  AtCommandHandler *commands[2] = {&commandHandlerMock.get(), &commandHandlerMock2.get()};

  AtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+XY\r\nAT+AB\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(1));
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock2), run)).Exactly(1));
  TEST_ASSERT_EQUAL_STRING(buffer, "+AB");
}

void it_ignores_garbage(void) {
  Mock<AtCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  AtCommandHandler *commands[1] = {&commandHandlerMock.get()};
  AtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, " ds87 fsdf77 +f AT+XY\r\nddddd");
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  resetSerial();
  feedInput(parser, "ddVAcdddd\r\nddddAT+XY\r\n");
  TEST_ASSERT_SERIAL_PRINT("OK\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(2));
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(it_executes_the_run_command);
  RUN_TEST(it_supports_the_write_command);
  RUN_TEST(it_ignores_unknown_commands);
  RUN_TEST(it_supports_the_write_command_with_passthrough);
  RUN_TEST(it_supports_the_write_command_with_empty_parameters);
  RUN_TEST(it_executes_the_base_commands);
  RUN_TEST(it_can_run_multiple_commands);
  RUN_TEST(it_ignores_garbage);
  return UNITY_END();
}

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) { return runUnityTests(); }

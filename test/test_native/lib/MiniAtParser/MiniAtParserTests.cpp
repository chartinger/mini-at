#include <ArduinoFake.h>
#include "unity.h"

#include "MiniAtParserTestHelper.hpp"

using namespace fakeit;

#include "MiniAtParser.hpp"

Serial_* MockSerial = ArduinoFakeMock(Serial);

const char* OK_RESPONSE = "\r\nOK\r\n";
const char* ERROR_RESPONSE = "\r\nERROR\r\n";

void resetSerial() {
  ArduinoFake(Serial).Reset();
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char*))).AlwaysReturn(1);
}

void setUp(void) {
  ArduinoFakeReset();
  resetSerial();
}

void tearDown(void) {}

void feedInput(MiniAtParser& parser, const char* data) {
  int length = strnlen(data, 100);
  for (int i = 0; i < length; i++) {
    parser.parse(data[i]);
  }
}

void it_executes_the_run_command(void) {
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};
  MiniAtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+XY\r\n");
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(1));
}

void it_ignores_unknown_commands(void) {
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};
  MiniAtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+WTF\r\n");
  TEST_ASSERT_SERIAL_PRINT(ERROR_RESPONSE);
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(0));
}

void it_supports_the_write_command(void) {
  MiniAtParser parser;
  char buffer[100];
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, write)).AlwaysDo([](Stream* out_stream, char** argv, uint16_t argc) {
    TEST_ASSERT_EQUAL_INT(4, argc);
    TEST_ASSERT_EQUAL_STRING("a", argv[0]);
    TEST_ASSERT_EQUAL_STRING("b", argv[1]);
    TEST_ASSERT_EQUAL_STRING("cc", argv[2]);
    TEST_ASSERT_EQUAL_STRING("dd", argv[3]);
    return 0;
  });
  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+XY=a,b,\"cc\",dd\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
}

void it_supports_the_write_command_with_empty_parameters(void) {
  MiniAtParser parser;
  char buffer[100];
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, write)).AlwaysDo([](Stream* out_stream, char** argv, uint16_t argc) {
    TEST_ASSERT_EQUAL_INT(4, argc);
    TEST_ASSERT_EQUAL_STRING("", argv[0]);
    TEST_ASSERT_EQUAL_STRING("", argv[1]);
    TEST_ASSERT_EQUAL_STRING("X", argv[2]);
    TEST_ASSERT_EQUAL_STRING("", argv[3]);
    return 0;
  });
  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+XY=,,X,\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
}

void it_supports_the_write_command_with_passthrough(void) {
  MiniAtParser parser;
  char buffer[100];
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, write)).AlwaysDo([](Stream* out_stream, char** argv, uint16_t argc) {
    TEST_ASSERT_EQUAL_INT(1, argc);
    return 10;
  });
  When(Method(commandHandlerMock, passthrough)).AlwaysDo([](Stream* out_stream, char* data, uint16_t dataLength) {
    TEST_ASSERT_EQUAL_INT(10, dataLength);
    TEST_ASSERT_EQUAL_STRING("Hello You!", data);
    return 0;
  });
  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  TEST_ASSERT_MOCK(feedInput(parser, "AT+XY=4\r\n"));
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(">\r\n");
  resetSerial();
  TEST_ASSERT_MOCK(feedInput(parser, "Hello You!\r\n"));
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), passthrough)).Exactly(1));
}

void it_executes_the_base_commands(void) {
  MiniAtParser parser;
  char buffer[100];
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  When(Method(commandHandlerMock, read)).AlwaysReturn(0);
  When(Method(commandHandlerMock, write)).AlwaysReturn(0);
  When(Method(commandHandlerMock, test)).AlwaysReturn(0);
  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));

  // RUN
  feedInput(parser, "AT+XY\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
  resetSerial();

  // READ
  TEST_ASSERT_MOCK(feedInput(parser, "AT+XY?\r\n"));
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), read)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
  resetSerial();

  // TEST
  feedInput(parser, "AT+XY=?\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), test)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
  resetSerial();

  // WRITE
  feedInput(parser, "AT+XY=\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
  resetSerial();
}

void it_can_run_multiple_commands(void) {
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  Mock<MiniAtParserCommandHandler> commandHandlerMock2;
  When(Method(commandHandlerMock2, getName)).AlwaysReturn("+AB");
  When(Method(commandHandlerMock2, run)).AlwaysReturn(0);

  MiniAtParserCommandHandler* commands[2] = {&commandHandlerMock.get(), &commandHandlerMock2.get()};

  MiniAtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+XY\r\nAT+AB\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(1));
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock2), run)).Exactly(1));
  TEST_ASSERT_EQUAL_STRING(buffer, "+AB");
}

void it_ignores_garbage(void) {
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);
  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};
  MiniAtParser parser;
  char buffer[100];
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, " ds87 fsdf77 +f AT+XY\r\nddddd");
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
  resetSerial();
  feedInput(parser, "ddVAcdddd\r\nddddAT+XY\r\n");
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), run)).Exactly(2));
}

void it_returns_an_error_if_passthrough_size_is_bigger_or_equal_the_buffer_size(void) {
  MiniAtParser parser;
  char buffer[100];
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+XY");
  When(Method(commandHandlerMock, write)).AlwaysReturn(100);

  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};

  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  TEST_ASSERT_MOCK(feedInput(parser, "AT+XY=4\r\n"));
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(ERROR_RESPONSE);
}

void it_errors_if_buffer_is_too_small_for_command(void) {
  MiniAtParser parser;
  char buffer[5];
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+ABCDEFG");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);

  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};

  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  TEST_ASSERT_MOCK(feedInput(parser, "AT+ABCDEFG\r\n"));
  TEST_ASSERT_SERIAL_PRINT(ERROR_RESPONSE);
}


void mytest(void) {
  MiniAtParser parser;
  char buffer[9];
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+ABCDEFG");
  When(Method(commandHandlerMock, run)).AlwaysReturn(0);

  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};

  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  TEST_ASSERT_MOCK(feedInput(parser, "AT+ABCDEFG\r\n"));
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
}

void mytest2(void) {
  MiniAtParser parser;
  char buffer[9];
  Mock<MiniAtParserCommandHandler> commandHandlerMock;
  When(Method(commandHandlerMock, getName)).AlwaysReturn("+ABCDE");
  When(Method(commandHandlerMock, write)).AlwaysDo([](Stream* out_stream, char** argv, uint16_t argc) {
    TEST_ASSERT_EQUAL_INT(1, argc);
    TEST_ASSERT_EQUAL_STRING("12345678", argv[0]);
    return 0;
  });
  MiniAtParserCommandHandler* commands[1] = {&commandHandlerMock.get()};
  parser.begin(MockSerial, commands, sizeof(commands), buffer, sizeof(buffer));
  feedInput(parser, "AT+ABCDE=\"12345678\"\r\n");
  TEST_ASSERT_MOCK(Verify(Method((commandHandlerMock), write)).Exactly(1));
  TEST_ASSERT_SERIAL_PRINT(OK_RESPONSE);
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

  RUN_TEST(it_returns_an_error_if_passthrough_size_is_bigger_or_equal_the_buffer_size);

  RUN_TEST(it_errors_if_buffer_is_too_small_for_command);
  RUN_TEST(mytest);
  RUN_TEST(mytest2);

  return UNITY_END();
}

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) {
  return runUnityTests();
}

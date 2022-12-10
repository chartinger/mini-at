#include "unity.h"
#include <Arduino.h>
using namespace fakeit;

#include "../src/at-command/AtParser.hpp"

struct SomeInterface {
  virtual int run() = 0;
};

// Mock<SomeInterface> mock;
// SomeInterface &i = mock.get();

// AT_COMMAND_RETURN_TYPE runCapture(AtParser *sender) {
//   i.run();
//   return 0;
// }

Mock<SomeInterface> *mock;

boolean wasRun = false;

void setUp(void) {
  ArduinoFakeReset();
  wasRun = false;
}

void tearDown(void) {
  // clean stuff up here
}

AT_COMMAND_RETURN_TYPE asss(AtParser *sender) { TEST_ASSERT_TRUE(false); }

void test_function_should_doBlahAndBlah(void) {
  Serial_ *clientMock = ArduinoFakeMock(Serial);
  mock = new Mock<SomeInterface>();
  // static Mock<SomeInterface> mock;
  // static SomeInterface &i = mock.get();
  at_command_t commands[] = {
      // {"+XY", asss, nullptr, nullptr, nullptr, nullptr},
      // {"", asss, nullptr, nullptr, nullptr, nullptr},
      {"+XY",
       [](AtParser *parser) -> AT_COMMAND_RETURN_TYPE {
         wasRun = true;
         return 0;
       },
       nullptr, nullptr, nullptr, nullptr},
  };
  AtParser parser;
  char buffer[100];
  parser.begin(clientMock, commands, sizeof(commands), buffer);
  // TEST_PRINTF("String    %s\n", "My string");
  TEST_ASSERT_EQUAL_INT(parser.getState(), PREFIX_A);
  parser.parse('A');
  TEST_ASSERT_EQUAL_INT(parser.getState(), PREFIX_T);
  parser.parse('T');
  // TEST_ASSERT_EQUAL_INT(parser.getState(), COMMAND);
  parser.parse('+');
  // TEST_ASSERT_EQUAL_INT(parser.getState(), COMMAND);
  parser.parse('X');
  parser.parse('Y');
  parser.parse('\r');
  // Verify(Method((*mock), run));
  TEST_ASSERT_EQUAL_STRING(buffer, "+XY");
  TEST_ASSERT(wasRun);
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

void test_function_should_doAlsoDoBlah(void) { TEST_ASSERT_TRUE(true); }

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_function_should_doBlahAndBlah);
  RUN_TEST(test_function_should_doAlsoDoBlah);
  return UNITY_END();
}

// WARNING!!! PLEASE REMOVE UNNECESSARY MAIN IMPLEMENTATIONS //

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) { return runUnityTests(); }

/**
 * For Arduino framework
 */
void setup() {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}
void loop() {}

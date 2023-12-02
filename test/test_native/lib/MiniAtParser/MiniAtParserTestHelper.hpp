#ifndef MINI_AT_PARSER_TEST_HELPER
#define MINI_AT_PARSER_TEST_HELPER

#include <ArduinoFake.h>
#include "unity.h"

using namespace fakeit;

#define TEST_ASSERT_MOCK(message)                    \
  try {                                              \
    message;                                         \
  } catch (const VerificationException ex) {         \
    TEST_FAIL_MESSAGE(ex.what());                    \
  } catch (const UnexpectedMethodCallException ex) { \
    TEST_FAIL_MESSAGE(ex.what().c_str());            \
  }

#define TEST_ASSERT_SERIAL_PRINT(message) \
  TEST_ASSERT_MOCK(Verify(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char*)).Using(StrEq((const char *)message))).Once());

#endif

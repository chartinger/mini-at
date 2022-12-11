#ifndef AT_TEST_HELPER_HPP
#define AT_TEST_HELPER_HPP

#include "unity.h"
#include <ArduinoFake.h>

using namespace fakeit;

#define TEST_ASSERT_MOCK(message)                                                                                                          \
  try {                                                                                                                                    \
    message;                                                                                                                               \
  } catch (const VerificationException ex) {                                                                                               \
    TEST_FAIL_MESSAGE(ex.what());                                                                                                          \
  } catch (const UnexpectedMethodCallException ex) {                                                                                       \
    TEST_FAIL_MESSAGE(ex.what().c_str());                                                                                                  \
  }

#define TEST_ASSERT_SERIAL_PRINT(message)                                                                                                  \
  TEST_ASSERT_MOCK(Verify(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char *)).Using(message)).Once());

struct CommandHandlerProxy {
  virtual int run() = 0;
  virtual int test() = 0;
  virtual int read() = 0;
  virtual int write() = 0;
  virtual int passthrough() = 0;
};

#endif

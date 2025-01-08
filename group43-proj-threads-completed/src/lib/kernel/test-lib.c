#include <test-lib.h>
#include <debug.h>
#include <stdio.h>

/* Global variable overridden by each test so that functions
   in this library can print test names */
const char* test_name = NULL;

/* Prints FORMAT as if with printf(),
   prefixing the output by the name of the test
   and following it with a new-line character. */
void msg(const char* format, ...) {
  va_list args;

  printf("(%s) ", test_name);
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  putchar('\n');
}

/* Prints failure message FORMAT as if with printf(),
   prefixing the output by the name of the test and FAIL:
   and following it with a new-line character,
   and then panics the kernel. */
void fail(const char* format, ...) {
  va_list args;

  printf("(%s) FAIL: ", test_name);
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  putchar('\n');

  PANIC("test failed");
}

/* Prints a message indicating the current test passed. */
void pass(void) { printf("(%s) PASS\n", test_name); }

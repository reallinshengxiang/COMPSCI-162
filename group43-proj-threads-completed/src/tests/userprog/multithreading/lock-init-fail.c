/* Ensure lock init on a bad address fails */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>

void test_main(void) {
  if (lock_init(NULL))
    fail("Initialized NULL lock");
  else
    msg("Success!");
}

/* Ensure sema init on a bad address and on a negative value fails */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>

void test_main(void) {
  // Test semaphore init on NULL sema
  if (sema_init(NULL, 0))
    fail("Initialized NULL sema");
  else
    msg("Success! NULL initialization prevented");

  // Test semaphore init on negative value
  sema_t sema;
  if (sema_init(&sema, -1))
    fail("Initialized negative sema");
  else
    msg("Success! Negative initialized prevented");
}

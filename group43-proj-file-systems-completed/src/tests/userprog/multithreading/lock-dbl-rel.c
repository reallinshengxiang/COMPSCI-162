/* Ensures double release fails */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>

void test_main(void) {
  lock_t lock;
  lock_check_init(&lock);
  lock_acquire(&lock);
  lock_release(&lock);
  msg("First release succeeded");
  lock_release(&lock);
  fail("Double release succeeded");
}

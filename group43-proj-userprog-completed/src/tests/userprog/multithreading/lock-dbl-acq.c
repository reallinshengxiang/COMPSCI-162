/* Ensures double acquire fails */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>

void test_main(void) {
  lock_t lock;
  lock_check_init(&lock);
  lock_acquire(&lock);
  msg("First acquire succeeded");
  lock_acquire(&lock);
  fail("Double acquire succeeded");
}

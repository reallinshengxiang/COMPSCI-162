/* Tests lock init, acquire, and release don't error */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>

void test_main(void) {
  lock_t lock;
  lock_check_init(&lock);
  lock_acquire(&lock);
  lock_release(&lock);
  msg("PASS");
}

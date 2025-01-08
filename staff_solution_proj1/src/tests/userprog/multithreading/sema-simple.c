/* Tests sema init, down, and up don't error */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>

void test_main(void) {
  sema_t sema;
  sema_check_init(&sema, 0);
  sema_up(&sema);
  sema_down(&sema);
  msg("PASS");
}

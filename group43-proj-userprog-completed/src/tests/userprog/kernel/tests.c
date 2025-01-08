#include "tests/userprog/kernel/tests.h"
#include <test-lib.h>
#include <debug.h>
#include <string.h>

static const struct test userprog_tests[] = {
    {"fp-kasm", test_fp_kasm},
    {"fp-kinit", test_fp_kinit},
};

/* Runs the userprog test named NAME. */
void run_userprog_test(const char* name) {
  const struct test* t;

  for (t = userprog_tests; t < userprog_tests + sizeof userprog_tests / sizeof *userprog_tests; t++)
    if (!strcmp(name, t->name)) {
      test_name = name;
      msg("begin");
      t->function();
      msg("end");
      return;
    }
  PANIC("no test named \"%s\"", name);
}

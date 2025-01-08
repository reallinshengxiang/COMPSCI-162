/* Verifies that the compute_e system call is implemented correctly */

#include <float.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void test_main(void) {
  test_name = "fp-kernel-e";
  msg("Computing e...");
  double e_res = compute_e(10);
  if (abs_val(e_res - E_VAL) < TOL) {
    msg("Success!");
    exit(162);
  } else {
    msg("Got e=%f, expected e=%f", e_res, E_VAL);
    exit(126);
  }
}

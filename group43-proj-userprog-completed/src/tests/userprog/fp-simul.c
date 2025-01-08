/* Executes compute-e userprog and computes an approximation of e itself
   (as a userprog) simultaneously.
   Ensures that floating point registers are saved on a context switch. */

#include <float.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void test_main(void) {
  test_name = "fp-simul";
  msg("Computing e...");
  pid_t e_pid = exec("compute-e");
  double e_res = sum_to_e(10);
  wait(e_pid);
  if (abs_val(e_res - E_VAL) < TOL) {
    msg("Success!");
    exit(162);
  } else {
    msg("Got e=%f, expected e=%f", e_res, E_VAL);
    exit(126);
  }
}

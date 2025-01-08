/* Child process run by floating point tests
   Uses Taylor definition to compute the Euler constant e */

#include <float.h>
#include "tests/lib.h"

int main(void) {
  test_name = "compute-e";
  double e_res = sum_to_e(10);
  if (abs_val(e_res - E_VAL) < TOL) {
    msg("Success!");
    exit(162);
  } else {
    msg("Got e=%f, expected e=%f", e_res, E_VAL);
    exit(126);
  }
}

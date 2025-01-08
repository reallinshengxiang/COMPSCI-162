/* Pushes to FPU registers, then exec's a process which does the same.
   Ensures that the exec'd process' FPU does not interfere with ours. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

#define NUM_VALUES 4
static int values[NUM_VALUES] = {1, 6, 2, 162};

void test_main(void) {
  test_name = "fp-asm";
  msg("Starting...");
  push_values_to_fpu(values, NUM_VALUES);
  wait(exec("fp-asm-helper"));
  if (pop_values_from_fpu(values, NUM_VALUES))
    exit(162);
  else
    exit(126);
}

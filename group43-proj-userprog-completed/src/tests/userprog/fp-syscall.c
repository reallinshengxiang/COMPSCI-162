/* Pushes values to the FPU, then jumps into the kernel to perform a
   system call (which uses floating point arithmetic).
   Ensures that floating point registers are saved on a system call. */

#include <float.h>
#include <stdint.h>
#include <string.h>
#include <syscall-nr.h>
#include "tests/lib.h"
#include "tests/main.h"

#define FPU_SIZE 108
#define NUM_VALUES 8
static int values[NUM_VALUES] = {1, 6, 2, 162, 126, 2, 6, 1};

/* Invokes syscall NUMBER, passing argument ARG0, and returns the
   return value as an `int'. */
#define syscall1(NUMBER, ARG0)                                                                     \
  ({                                                                                               \
    int retval;                                                                                    \
    asm volatile("pushl %[arg0]; pushl %[number]; int $0x30; addl $8, %%esp"                       \
                 : "=a"(retval)                                                                    \
                 : [number] "i"(NUMBER), [arg0] "g"(ARG0)                                          \
                 : "memory");                                                                      \
    retval;                                                                                        \
  })

void test_main(void) {
  test_name = "fp-syscall";
  uint8_t fpu_before[FPU_SIZE];
  uint8_t fpu_after[FPU_SIZE];

  msg("Computing e...");
  push_values_to_fpu(values, NUM_VALUES);

  // Manually call the system call so that the compiler does not
  // generate FP instructions that modify the FPU in user space
  // Save FPU state before and after the syscall
  asm("fsave (%0)" : : "g"(&fpu_before));
  int e_res = syscall1(SYS_COMPUTE_E, 10);
  asm("fsave (%0)" : : "g"(&fpu_after));

  // Check if the FPU state is the same before and after the syscall
  // Ignore the Control Word (bytes 0-4) and the Tag Word (bytes 8-12)
  // since those are modified by the FSAVE instruction
  compare_bytes(&fpu_before[28], &fpu_after[28], FPU_SIZE - 28, 0, test_name);
  if (pop_values_from_fpu(values, NUM_VALUES)) {
    msg("Success!");
  } else {
    msg("Incorrect values popped");
    exit(126);
  }

  // Convert the integer returned by the system call into a proper
  // double, so we can see if the return value is correct
  float e_res_flt;
  ASSERT(sizeof(float) == sizeof(int));
  memcpy(&e_res_flt, &e_res, sizeof(int));
  double e_res_dbl = (double)e_res_flt;
  if (abs_val(e_res_dbl - E_VAL) < TOL) {
    msg("Kernel computation successful");
    exit(162);
  } else {
    msg("Got e=%f, expected e=%f", e_res_dbl, E_VAL);
    exit(261);
  }
}

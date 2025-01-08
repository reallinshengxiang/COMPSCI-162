/* Ensures that FPU is reset when jumping into a user program */

#include <stdint.h>
#include "tests/lib.h"
#include "tests/main.h"

#define FPU_SIZE 108

void test_main(void) {
  test_name = "fp-init";
  uint8_t fpu[FPU_SIZE];
  uint8_t init_fpu[FPU_SIZE];
  asm("fsave (%0); fninit; fsave (%1)" : : "g"(&fpu), "g"(&init_fpu));
  compare_bytes(&fpu, &init_fpu, FPU_SIZE, 0, test_name);
  msg("Success!");
  exit(162);
}

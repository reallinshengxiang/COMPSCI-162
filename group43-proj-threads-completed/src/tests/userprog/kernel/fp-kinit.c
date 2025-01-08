/* Tests whether floating point initialized when new threads are made */

#include <float.h>
#include <stdint.h>
#include <string.h>
#include <debug.h>
#include "tests/userprog/kernel/tests.h"
#include "threads/thread.h"
#include "threads/synch.h"

#define FPU_SIZE 108
#define NUM_VALUES 8
static int values[NUM_VALUES] = {1, 6, 2, 162, 126, 2, 6, 1};
struct semaphore check_done;

void fpu_init_check(void*);

void fpu_init_check(void* args UNUSED) {
  uint8_t fpu[FPU_SIZE];
  uint8_t init_fpu[FPU_SIZE];
  asm("fsave (%0); fninit; fsave (%1)" : : "g"(&fpu), "g"(&init_fpu));
  if (memcmp(&fpu, &init_fpu, FPU_SIZE))
    fail("FPU not initialized correctly");
  sema_up(&check_done);
}

void test_fp_kinit(void) {
  sema_init(&check_done, 0);
  for (int i = 0; i < NUM_VALUES; i++)
    fpu_push(values[i]);
  thread_create("fpu-init-chk", PRI_DEFAULT, &fpu_init_check, NULL);
  sema_down(&check_done);
  pass();
}

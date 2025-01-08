/* Tests whether floating point registers are saved on kernel transitions */

#include <float.h>
#include "tests/userprog/kernel/tests.h"
#include "threads/thread.h"
#include "threads/synch.h"

#define NUM_VALUES 4
int values[NUM_VALUES] = {100, 101, 102, 103};

void fpu_push_up(void* args);
int fpu_push_pop(int x);

struct fpu_args {
  int push_val;
  struct semaphore push_done;
};

/* Pushes to the FPU its argument and ups a semaphore */
void fpu_push_up(void* args) {
  struct fpu_args* fpuargs = (struct fpu_args*)args;
  fpu_push(fpuargs->push_val);
  sema_up(&fpuargs->push_done);
}

/* Pushes integer x to the FPU, yields the CPU, then pops it */
int fpu_push_pop(int x) {
  fpu_push(x);

  // Yield CPU and wait for thread to finish
  struct fpu_args fpuargs;
  fpuargs.push_val = x + 1;
  sema_init(&fpuargs.push_done, 0);
  thread_create("fpu-pusher", PRI_DEFAULT, &fpu_push_up, (void*)&fpuargs);
  sema_down(&fpuargs.push_done);

  return fpu_pop();
}

void test_fp_kasm(void) {
  for (int i = 0; i < NUM_VALUES; i++) {
    int curr = values[i];
    int curr_pop = fpu_push_pop(curr);
    if (curr != curr_pop)
      msg("Error: pushed %d but popped %d", curr, curr_pop);
  }
  pass();
}

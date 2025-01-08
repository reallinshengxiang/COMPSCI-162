#ifndef __LIB_FLOAT_H
#define __LIB_FLOAT_H

#define E_VAL 2.718281
#define TOL 0.000002

/* Pushes integer num to the FPU */
static inline void fpu_push(int num) {
  asm volatile("pushl %0; flds (%%esp); addl $4, %%esp" : : "m"(num));
}

/* Pops integer from the FPU */
static inline int fpu_pop(void) {
  int val;
  asm volatile("subl $4, %%esp; fstps (%%esp); mov (%%esp), %0; addl $4, %%esp"
               : "=r"(val)
               :
               : "memory");
  return val;
}

/* Stores a clean copy of the FPU to a 108B memory location DEST.
   Uses a 108B memory location BUF as a temporary storage */
static inline void fpu_save_init(void* dest, void* buf) {
  asm volatile("fsave (%0); fninit; fsave (%1); frstor (%2)" : : "r"(buf), "r"(dest), "r"(buf));
}

int sys_sum_to_e(int);
double sum_to_e(int);
double abs_val(double);

#endif /* lib/debug.h */

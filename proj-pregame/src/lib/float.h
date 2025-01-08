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

int sys_sum_to_e(int);
double sum_to_e(int);
double abs_val(double);

#endif /* lib/debug.h */

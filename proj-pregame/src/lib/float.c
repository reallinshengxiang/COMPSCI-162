#include <float.h>
#include <debug.h>
#include <string.h>

int factorial(int);

/* Computes the absolute value of x, |x| */
double abs_val(double x) { return x > 0 ? x : -x; }

/* Computes n factorial, for any n >= 0 */
int factorial(int n) {
  int total = 1;
  for (int i = 2; i <= n; i++) {
    total *= i;
  }
  return total;
}

/* Computes the first n + 1 terms of the Taylor summation of
   the Euler constant e */
double sum_to_e(int n) {
  double e = 0.0;
  for (int i = 0; i <= n; i++) {
    e += 1.0 / factorial(i);
  }
  return e;
}

/* Computes sum_to_e(n) and marshalls output argument as an integer
   instead of a floating point value, since that is what the kernel
   syscall interface expects */
int sys_sum_to_e(int n) {
  double res = sum_to_e(n);
  float conv_res = (float)res;
  int ret_val;
  ASSERT(sizeof(float) == sizeof(int));
  memcpy(&ret_val, &conv_res, sizeof(float));
  return ret_val;
}

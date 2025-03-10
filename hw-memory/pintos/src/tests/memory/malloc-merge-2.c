/* Test that free blocks are merged. */

#include <stdlib.h>
#include <string.h>
#include "tests/lib.h"
#include "tests/main.h"

#define NUM_INTS (1 << 12)

static void check_zero(int* buffer, int len) {
  for (int i = 0; i != len; i++) {
    ASSERT(buffer[i] == 0);
  }
}

static void fill(int* buffer, int len) {
  for (int i = 0; i != len; i++) {
    buffer[i] = 0x05158E57;
  }
}

static void* test_alloc(int num_ints) {
  int* ptr = calloc(num_ints, sizeof(int));
  check_zero(ptr, num_ints);
  fill(ptr, num_ints);
  return ptr;
}

void test_main(void) {
  int* p = test_alloc(2 * NUM_INTS);
  int* q = test_alloc(2 * NUM_INTS);
  int* r = test_alloc(2 * NUM_INTS);
  int* s = test_alloc(2 * NUM_INTS);

  free(r);
  free(s);
  free(p);
  free(q);

  int* t = test_alloc(3 * NUM_INTS);

  ASSERT(p <= t && t < r);

  free(t);
}

int main(int argc UNUSED, char* argv[] UNUSED) {
  test_name = "malloc-merge-2";
  msg("begin");
  test_main();
  msg("end");
  return 0;
}

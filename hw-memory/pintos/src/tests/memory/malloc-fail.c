/* Test that attempts to malloc huge memory fail. */

#include <stdlib.h>
#include <string.h>
#include "tests/lib.h"
#include "tests/main.h"

#define GIBI (1 << 30)
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
  int* p = test_alloc(NUM_INTS);
  int* q = calloc(GIBI, 1);
  ASSERT(q == NULL);
  int* r = test_alloc(NUM_INTS);

  ASSERT(p != r);
  ASSERT(p + NUM_INTS <= r || r + NUM_INTS <= p);

  free(p);
  free(r);
}

int main(int argc UNUSED, char* argv[] UNUSED) {
  test_name = "malloc-fail";
  msg("begin");
  test_main();
  msg("end");
  return 0;
}

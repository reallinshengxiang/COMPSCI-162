#include "tests/lib.h"
#include "sbrk-simple.h"

int main(int argc UNUSED, char* argv[] UNUSED) {
  test_name = "sbrk-page";
  msg("begin");
  test_sbrk(4096);
  msg("end");
  return 0;
}

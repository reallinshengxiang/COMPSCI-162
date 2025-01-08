/* Does absolutely nothing. */

#include "tests/lib.h"

int main(int argc, char* argv[] UNUSED) {
  register unsigned int esp asm("esp");
  if (argc != 1)  // argc must be 1.
    return -1;
  else
    return esp % 16;
}

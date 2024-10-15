#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void test_main(void) {
  int fd;
  char buffer[16];

  CHECK(create("testfile.txt", 20), "create 'testfile.txt'");

  fd = open("testfile.txt");
  CHECK(fd > 1, "open 'testfile.txt'");

  write(fd, "This is a test file.", 20);

  close(fd);

  fd = open("testfile.txt");
  CHECK(fd > 1, "reopen 'testfile.txt'");

  CHECK(tell(fd) == 0, "tell at position 0");

  seek(fd, 10);
  CHECK(tell(fd) == 10, "tell after seek to position 10");

  read(fd, buffer, 5);
  CHECK(tell(fd) == 15, "tell after read 5 bytes");

  seek(fd, 5);
  CHECK(tell(fd) == 5, "tell after seek to position 5");

  seek(fd, 20);
  CHECK(tell(fd) == 20, "tell at the end of the file");
}
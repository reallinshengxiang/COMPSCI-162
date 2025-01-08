
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"
#define FILE_SIZE 64 * 1024
static char buf_a[FILE_SIZE];
static char buf_b[FILE_SIZE];
static void write_bytes(const char* file_name, int fd, const char* buf, size_t* ofs) {
  if (*ofs < FILE_SIZE) {
    size_t ret_val;
    ret_val = write(fd, buf + *ofs, 4);
    *ofs += 4;
  }
}
static void read_bytes(const char* file_name, int fd, const char* buf, size_t* ofs) {
  if (*ofs < FILE_SIZE) {
    size_t ret_val;
    ret_val = read(fd, buf + *ofs, 4);
    *ofs += 4;
  }
}
void test_main() {
  int fd_a;
  size_t ofs_a = 0;
  random_init(0);
  random_bytes(buf_a, sizeof buf_a);
  CHECK(create("a", 0), "create \"a\"");
  CHECK((fd_a = open("a")) > 1, "open \"a\"");
  read_bytes("a", fd_a, buf_a, &ofs_a);
  msg("Checking block statistics after reading \"a\" ");
  stats();
  close(fd_a);
  CHECK((fd_a = open("a")) > 1, "re-open \"a\"");
  ofs_a = 0;
  read_bytes("a", fd_a, buf_a, &ofs_a);
  msg("Checking block statistics after re-reading \"a\" ");
  stats();
  msg("close \"a\"");
  close(fd_a);
  return;
}
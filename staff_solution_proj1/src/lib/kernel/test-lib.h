#ifndef __LIB_KERNEL_TEST_LIB_H
#define __LIB_KERNEL_TEST_LIB_H

typedef void test_func(void);

extern const char* test_name;

struct test {
  const char* name;
  test_func* function;
};

void msg(const char*, ...);
void fail(const char*, ...);
void pass(void);

#endif /* lib/kernel/test-lib.h */

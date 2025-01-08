#ifndef TESTS_USERPROG_KERNEL_TESTS_H
#define TESTS_USERPROG_KERNEL_TESTS_H

#include <test-lib.h>

void run_userprog_test(const char*);

extern test_func test_fp_kasm;
extern test_func test_fp_kinit;

#endif /* tests/userprog/kernel/tests.h */

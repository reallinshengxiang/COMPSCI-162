#ifndef TESTS_THREADS_TESTS_H
#define TESTS_THREADS_TESTS_H

#include <test-lib.h>

void run_threads_test(const char*);

extern test_func test_alarm_single;
extern test_func test_alarm_multiple;
extern test_func test_alarm_simultaneous;
extern test_func test_alarm_priority;
extern test_func test_alarm_zero;
extern test_func test_alarm_negative;
extern test_func test_priority_change;
extern test_func test_priority_donate_one;
extern test_func test_priority_donate_multiple;
extern test_func test_priority_donate_multiple2;
extern test_func test_priority_donate_sema;
extern test_func test_priority_donate_nest;
extern test_func test_priority_donate_lower;
extern test_func test_priority_donate_chain;
extern test_func test_priority_fifo;
extern test_func test_priority_preempt;
extern test_func test_priority_sema;
extern test_func test_priority_condvar;
extern test_func test_priority_starve;
extern test_func test_priority_starve_sema;
extern test_func test_mt_matmul_1;
extern test_func test_mt_matmul_2;
extern test_func test_mt_matmul_4;
extern test_func test_mt_matmul_16;
extern test_func test_mlfqs_load_1;
extern test_func test_mlfqs_load_60;
extern test_func test_mlfqs_load_avg;
extern test_func test_mlfqs_recent_1;
extern test_func test_mlfqs_fair_2;
extern test_func test_mlfqs_fair_20;
extern test_func test_mlfqs_nice_2;
extern test_func test_mlfqs_nice_10;
extern test_func test_mlfqs_block;
extern test_func test_smfs_starve_0;
extern test_func test_smfs_starve_1;
extern test_func test_smfs_starve_2;
extern test_func test_smfs_starve_4;
extern test_func test_smfs_starve_8;
extern test_func test_smfs_starve_16;
extern test_func test_smfs_starve_64;
extern test_func test_smfs_starve_256;
extern test_func test_smfs_prio_change;
extern test_func test_smfs_hierarchy_8;
extern test_func test_smfs_hierarchy_16;
extern test_func test_smfs_hierarchy_32;
extern test_func test_smfs_hierarchy_64;
extern test_func test_smfs_hierarchy_256;

#endif /* tests/threads/tests.h */

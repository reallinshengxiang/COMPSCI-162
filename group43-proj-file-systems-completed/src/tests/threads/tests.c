#include "tests/threads/tests.h"
#include <test-lib.h>
#include <debug.h>
#include <string.h>

static const struct test threads_tests[] = {
    {"alarm-single", test_alarm_single},
    {"alarm-multiple", test_alarm_multiple},
    {"alarm-simultaneous", test_alarm_simultaneous},
    {"alarm-priority", test_alarm_priority},
    {"alarm-zero", test_alarm_zero},
    {"alarm-negative", test_alarm_negative},
    {"priority-change", test_priority_change},
    {"priority-donate-one", test_priority_donate_one},
    {"priority-donate-multiple", test_priority_donate_multiple},
    {"priority-donate-multiple2", test_priority_donate_multiple2},
    {"priority-donate-nest", test_priority_donate_nest},
    {"priority-donate-sema", test_priority_donate_sema},
    {"priority-donate-lower", test_priority_donate_lower},
    {"priority-donate-chain", test_priority_donate_chain},
    {"priority-fifo", test_priority_fifo},
    {"priority-preempt", test_priority_preempt},
    {"priority-sema", test_priority_sema},
    {"priority-condvar", test_priority_condvar},
    {"priority-starve", test_priority_starve},
    {"priority-starve-sema", test_priority_starve_sema},
    {"st-matmul", test_mt_matmul_1},
    {"mt-matmul-2", test_mt_matmul_2},
    {"mt-matmul-4", test_mt_matmul_4},
    {"mt-matmul-16", test_mt_matmul_16},
    {"mlfqs-load-1", test_mlfqs_load_1},
    {"mlfqs-load-60", test_mlfqs_load_60},
    {"mlfqs-load-avg", test_mlfqs_load_avg},
    {"mlfqs-recent-1", test_mlfqs_recent_1},
    {"mlfqs-fair-2", test_mlfqs_fair_2},
    {"mlfqs-fair-20", test_mlfqs_fair_20},
    {"mlfqs-nice-2", test_mlfqs_nice_2},
    {"mlfqs-nice-10", test_mlfqs_nice_10},
    {"mlfqs-block", test_mlfqs_block},
    {"smfs-starve-0", test_smfs_starve_0},
    {"smfs-starve-1", test_smfs_starve_1},
    {"smfs-starve-2", test_smfs_starve_2},
    {"smfs-starve-4", test_smfs_starve_4},
    {"smfs-starve-8", test_smfs_starve_8},
    {"smfs-starve-16", test_smfs_starve_16},
    {"smfs-starve-64", test_smfs_starve_64},
    {"smfs-starve-256", test_smfs_starve_256},
    {"smfs-prio-change", test_smfs_prio_change},
    {"smfs-hierarchy-8", test_smfs_hierarchy_8},
    {"smfs-hierarchy-16", test_smfs_hierarchy_16},
    {"smfs-hierarchy-32", test_smfs_hierarchy_32},
    {"smfs-hierarchy-64", test_smfs_hierarchy_64},
    {"smfs-hierarchy-256", test_smfs_hierarchy_256}};

/* Runs the threads test named NAME. */
void run_threads_test(const char* name) {
  const struct test* t;

  for (t = threads_tests; t < threads_tests + sizeof threads_tests / sizeof *threads_tests; t++)
    if (!strcmp(name, t->name)) {
      test_name = name;
      msg("begin");
      t->function();
      msg("end");
      return;
    }
  PANIC("no test named \"%s\"", name);
}

/* Ensures that the fair scheduler gives higher-priority threads
   more CPU time on average than low-priority threads. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"
#include "threads/interrupt.h"

static thread_func counter_thread_func;

static void test_smfs_hierarchy(size_t num_threads);

#define TEST(n)                                                                                    \
  void test_smfs_hierarchy_##n(void) { test_smfs_hierarchy(n); }

TEST(8);
TEST(16);
TEST(32);
TEST(64);
TEST(256);

int counters[8];
char* thread_names[8] = {"t-min+00", "t-min+08", "t-min+16", "t-min+24",
                         "t-min+32", "t-min+40", "t-min+48", "t-min+56"};

static struct semaphore barrier_sema;
static bool keep_looping = true;

void test_smfs_hierarchy(size_t num_threads) {
  ASSERT(active_sched_policy == SCHED_FAIR);

  /* Not essential, but reduces randomness. */
  thread_set_priority(PRI_MAX);

  /* Stop all threads from running until we're done spawning. */
  sema_init(&barrier_sema, 0);
  barrier();

  /* Initialize counters. */
  for (size_t i = 0; i < 8; i++)
    counters[i] = 0;

  msg("Spawning counter threads...");

  size_t next_pri_offset = PRI_MIN + 8;
  for (size_t i = 0; i < num_threads; i++) {
    /* Create competitor threads, each with a priority equal to
       PRI_MIN + c, where c ∈ {0, 8, 16, ..., 56} */
    void* argument = (void*)&counters[next_pri_offset >> 3];
    char* name = thread_names[next_pri_offset >> 3];
    thread_create(name, PRI_MIN + next_pri_offset, counter_thread_func, argument);
    next_pri_offset = (next_pri_offset + 8) % 64;
  }

  msg("Done spawning counter threads.");

  /* Release barrier */
  barrier();
  for (size_t i = 0; i < num_threads; i++)
    sema_up(&barrier_sema);

  /* Sleep for a bit to allow the counter threads time to run amok. */
  timer_sleep(200000);

  /* Rein in the counters...
     Note that the order of acquisition gives higher-priority threads a
     slight advantage; this is nice for students, so I left it as-is. */
  for (size_t i = 0; i < 8; i++) {
    intr_disable();
    keep_looping = false;
    intr_enable();
  }

  for (size_t i = 0, j = 1; j < 8; i++, j++) {
    if (counters[i] < counters[j]) {
      msg("counters[%d] < counters[%d]", i, j);
    } else {
      msg("counters[%d] >= counters[%d]!!", i, j);
      msg("  Threads with priority...");
      msg("    ... %d ran %d times,", PRI_MIN + i * 8, counters[i]);
      msg("    ... %d ran %d times,", PRI_MIN + j * 8, counters[j]);
    }
  }
}

static void counter_thread_func(void* argument) {
  int* counter = (int*)argument;

  sema_down(&barrier_sema);

  bool loop = true;

  /* A very inefficient way of counting at each priority without
     atomics or having to write different code for each number
     of counter threads. */
  while (loop) {
    intr_disable();
    *counter += 1;
    loop = keep_looping;
    intr_enable();
  }
}

/* Ensures that the fair scheduler allows low-priority threads to
   make forward progress, even in the presence of high-priority
   threads. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

static thread_func starving_thread_func;
static thread_func greedy_thread_func;

static void test_smfs_starve(size_t competing_threads);

#define TEST(n)                                                                                    \
  void test_smfs_starve_##n(void) { test_smfs_starve(n); }

TEST(0);
TEST(1);
TEST(2);
TEST(4);
TEST(8);
TEST(16);
TEST(64);
TEST(256);

static struct semaphore barrier_sema;
struct semaphore sleep_sema;

void test_smfs_starve(size_t competing_threads) {
  ASSERT(active_sched_policy == SCHED_FAIR);

  /* Not essential, but reduces randomness. */
  thread_set_priority(PRI_MAX);

  /* Stop all threads from running until we're done spawning. */
  sema_init(&barrier_sema, 0);
  barrier();

  msg("Spawning competitor threads...");

  size_t next_priority = PRI_MIN + 8;
  for (size_t i = 0; i < competing_threads; i++) {
    /* Create competitor threads with priorities in the range
       [PRI_MIN + 8, PRI_MAX - 8] in increments of 1 */
    thread_create("competitor-thread", next_priority, greedy_thread_func, NULL);
    next_priority = next_priority < PRI_MAX - 8 ? next_priority + 1 : PRI_MIN + 8;
  }
  msg("Done spawning competitor threads.");

  /* Release barrier */
  barrier();
  for (size_t i = 0; i < competing_threads; i++)
    sema_up(&barrier_sema);

  /* Create low-priority thread */
  sema_init(&sleep_sema, 0);
  thread_create("starving-thread", PRI_MIN, starving_thread_func, NULL);

  /* Sleep until low-prio thread completes. */
  sema_down(&sleep_sema);
}

static void starving_thread_func(void* aux UNUSED) {
  //volatile uint32_t work = 1;
  //for (int i = 0; i < 16; i++) {
  //  work += i;
  //  if (work - 1) {
  //    work *= i + 1;
  //  }
  //}
  msg("Low-priority thread got to run!");
  sema_up(&sleep_sema);
}

static void greedy_thread_func(void* aux UNUSED) {
  volatile uint32_t state = 0xCCCCCCCC;

  sema_down(&barrier_sema);

  /* computes a 32-bit xorshift in a loop */
  while (true) {
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x;
  }

  __builtin_unreachable();
}

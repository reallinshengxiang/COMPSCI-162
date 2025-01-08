/* Ensures that the presence of a perpetually-busy high-priority
   thread induces total starvation in a lower-priority thread. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

static thread_func low_prio_thread_func;
static thread_func medium_prio_thread_func;

void test_priority_starve(void) {
  ASSERT(active_sched_policy == SCHED_PRIO);

  thread_set_priority(PRI_MIN + 16);
  thread_create("medium-priority", PRI_MIN + 8, medium_prio_thread_func, NULL);
  thread_create("low-priority", PRI_MIN, low_prio_thread_func, NULL);

  /* Note that this tick amount should complete in <4 seconds:
     if this test is timing out, it likely means your scheduler
     is incorrectly context-switching between the low- and medium-
     priority threads. */
  timer_sleep(4096);

  msg("The medium-priority thread has been preempted.");
  msg("Thread %s done!", thread_name());
}

static void low_prio_thread_func(void* aux UNUSED) {
  msg("The low-priority thread has been given a chance to run!");
  msg("Thread %s done!", thread_name());
}

static void medium_prio_thread_func(void* aux UNUSED) {
  volatile uint32_t state = 0xCCCCCCCC;

  msg("The medium-priority thread has been given a chance to run!");

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

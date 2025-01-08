/* Lowering a thread's priority so that it is no
   longer the highest-priority thread in the system no longer
   requires that it yield immediately! It should, however,
   be pre-empted eventually. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

static thread_func alice_thread;
static thread_func bob_thread;

struct semaphore alice_sema;
struct semaphore bob_sema;

void test_smfs_prio_change(void) {
  ASSERT(active_sched_policy == SCHED_FAIR);

  thread_set_priority(PRI_MAX);

  sema_init(&alice_sema, 0);
  sema_init(&bob_sema, 0);

  thread_create("alice", PRI_MIN + 32, alice_thread, NULL);

  thread_create("bob", PRI_MIN + 16, bob_thread, NULL);

  msg("Waking up alice.");
  sema_up(&alice_sema);

  timer_sleep(1024);

  msg("Thread %s exiting.", thread_name());
}

static void alice_thread(void* aux UNUSED) {
  sema_down(&alice_sema);
  msg("Thread %s woke up.", thread_name());

  msg("Waking up bob.", thread_name());
  sema_up(&bob_sema);

  thread_set_priority(PRI_MIN);
}

static void bob_thread(void* aux UNUSED) {
  sema_down(&bob_sema);
  msg("Thread %s woke up.", thread_name());
}

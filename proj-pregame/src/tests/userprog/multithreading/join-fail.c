/* Tests join failure scenarios */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

// Joiner thread data
struct joiner_thread_data {
  tid_t tid;
  bool should_succeed;
};

struct self_joiner_thread_data {
  tid_t self_tid;
  sema_t populate_sjtd;
};

// Global variables
sema_t thread_sema;

void thread_function(void* arg_);
void joiner_function(void* arg_);
void self_joiner_function(void* arg_);

/* Prints that it started then downs a semaphore */
void thread_function(void* arg_) {
  sema_t* sema = (sema_t*)arg_;
  sema_down(sema);
  msg("Thread finished");
}

/* Tries to join on a thread */
void joiner_function(void* arg_) {
  struct joiner_thread_data* jtd = (struct joiner_thread_data*)arg_;
  if (jtd->should_succeed)
    pthread_check_join(jtd->tid);
  else if (pthread_join(jtd->tid))
    fail("Should have failed.");
  msg("Finished joining");
}

/* Tries to join on itself */
void self_joiner_function(void* arg_) {
  struct self_joiner_thread_data* sjtd = (struct self_joiner_thread_data*)arg_;
  sema_down(&sjtd->populate_sjtd); // Wait until tells us our TID
  if (pthread_join(sjtd->self_tid))
    fail("Should have failed.");
  msg("Finished self joining");
}

void test_main(void) {
  syn_msg = true;
  msg("Main starting");

  // Initialize global sema
  sema_check_init(&thread_sema, 0);

  // Try to join on a non-existent thread
  if (pthread_join(1234))
    fail("Joined on non-existent thread");

  // Spawn a thread that hangs
  tid_t child_tid = pthread_check_create(thread_function, &thread_sema);

  // Spawn a thread that joins on the hanging thread
  struct joiner_thread_data jtd_success;
  jtd_success.tid = child_tid;
  jtd_success.should_succeed = true;
  tid_t joiner_tid = pthread_check_create(joiner_function, &jtd_success);

  // Up the hanging thread's semaphore so that it can proceed
  // Both it and the joiner should finish
  sema_up(&thread_sema);
  pthread_check_join(joiner_tid);
  msg("Main regained control");

  // Spawn another thread that joins on the hanging thread
  // This one should fail
  struct joiner_thread_data jtd_fail;
  jtd_fail.tid = child_tid;
  jtd_fail.should_succeed = false;
  pthread_check_join(pthread_check_create(joiner_function, &jtd_fail));
  msg("Main regained control");

  // Spawn a thread that tries to join on itself
  struct self_joiner_thread_data sjtd;
  sema_check_init(&sjtd.populate_sjtd, 0);
  tid_t self_tid = pthread_check_create(self_joiner_function, &sjtd);
  sjtd.self_tid = self_tid;
  sema_up(&sjtd.populate_sjtd);
  pthread_check_join(self_tid);

  msg("Main finishing");
  syn_msg = false;
}

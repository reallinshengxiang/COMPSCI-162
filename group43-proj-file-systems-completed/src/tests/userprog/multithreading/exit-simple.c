/* Make sure pthread_exit called by child thread actually terminates it */

#include "tests/lib.h"
#include "tests/main.h"
#include <pthread.h>
#include <debug.h>

void thread_function(void* arg);

/* Simple thread function that just prints PASS */
void thread_function(void* arg UNUSED) {
  msg("Thread ran.");
  pthread_exit();
  fail("Thread kept running?");
}

void test_main(void) {
  msg("Main started.");
  tid_t tid = pthread_check_create(thread_function, NULL);
  pthread_check_join(tid);
  msg("Main finished.");
}

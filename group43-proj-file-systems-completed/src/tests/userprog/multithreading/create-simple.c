/* Tests if new thread can be created, gracefully waited on, and exit */

#include "tests/lib.h"
#include "tests/main.h"
#include <pthread.h>
#include <debug.h>

void thread_function(void* arg);

/* Simple thread function that just prints PASS */
void thread_function(void* arg UNUSED) { msg("Thread ran."); }

void test_main(void) {
  msg("Main started.");
  tid_t tid = pthread_check_create(thread_function, NULL);
  pthread_check_join(tid);
  msg("Main finished.");
}

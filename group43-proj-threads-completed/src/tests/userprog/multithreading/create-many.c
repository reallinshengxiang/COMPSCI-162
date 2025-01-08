/* Tests if many thread can be created, gracefully waited on, and exit */

#include "tests/lib.h"
#include "tests/main.h"
#include <pthread.h>

#define NUM_THREADS 10

void thread_function(void* arg_);

/* Simple thread function that just prints its argument */
void thread_function(void* arg_) {
  int* arg = (int*)arg_;
  msg("Thread prints %d", *arg);
}

void test_main(void) {
  syn_msg = true;
  int arg = 162;
  tid_t tids[NUM_THREADS];

  // Spawn threads
  for (int i = 0; i < NUM_THREADS; i++)
    tids[i] = pthread_check_create(thread_function, &arg);

  // Wait on threads
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_check_join(tids[i]);

  msg("Main finished");
  syn_msg = false;
}

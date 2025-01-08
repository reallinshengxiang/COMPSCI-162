/* Spawn six threads which each access shared data, protected by a lock  */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

#define NUM_THREADS 6

lock_t lock;
int best_class;

void thread_function(void* arg_);

/* Each thread adds 1 to the shared data 27 times */
void thread_function(void* arg_) {
  int* arg = (int*)arg_;
  for (int i = 0; i < 27; i++) {
    lock_acquire(&lock);
    (*arg)++;
    lock_release(&lock);
  }
}

void test_main(void) {
  lock_check_init(&lock);

  // Initialize data. Lock acquire here isn't necessary, but oh well.
  lock_acquire(&lock);
  best_class = 0;
  lock_release(&lock);

  // Spawn threads
  tid_t tids[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
    tids[i] = pthread_check_create(thread_function, &best_class);

  // Wait on threads
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_check_join(tids[i]);

  // Check that the value is correct
  lock_acquire(&lock);
  int final = best_class;
  lock_release(&lock);

  if (final != 162)
    fail("The best class is 162, but you thought it was %d", final);
  msg("PASS");
}

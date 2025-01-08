/* Thread i spawns thread i + 1, waits for thread i + 1 to finish,
   and then continues. Waiting is managed by join.
   This is the exact same test as sema-wait, but manages waiting
   via pthread_join() instead of semaphores. */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

#define NUM_THREADS 10

void thread_function(void* arg_);

/* Each thread prints its thread number n, spawns a new thread with
   thread number n + 1, and waits for that thread to finish executing
   before this thread prints its thread number again and finishes */
void thread_function(void* arg_) {
  int* thread_num = (int*)arg_;
  msg("Thread %d starting", *thread_num);

  // Create child thread, and sleep until it finishes
  if (*thread_num < NUM_THREADS) {
    int child_thread_num = *thread_num + 1;
    pthread_check_join(pthread_check_create(thread_function, &child_thread_num));
  }

  // Child has finished, print that we finished then wake our parent
  msg("Thread %d finishing", *thread_num);
}

void test_main(void) {
  syn_msg = true;
  msg("Main starting");

  // Spawn the first thread and wait for it
  int first_thread_num = 1;
  pthread_check_join(pthread_check_create(thread_function, &first_thread_num));

  msg("Main finishing");
  syn_msg = false;
}

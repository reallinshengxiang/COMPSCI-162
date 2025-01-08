/* Creates a bunch of threads, then exits them, then creates that
   many threads again. Ensures resources are freed */

#include "tests/lib.h"
#include "tests/main.h"
#include <pthread.h>

#define NUM_THREADS 126

// Global variables
lock_t thread_hang, count_lock;
sema_t main_wait;
int created_threads;
char* first = "first";
char* second = "second";

void thread_function(void* arg_);

/* Hangs until main releases the global lock */
void thread_function(void* arg_) {
  char* str = (char*)arg_;

  // Safely wake up main once all the threads are created
  lock_acquire(&count_lock);
  if (++created_threads == NUM_THREADS)
    sema_up(&main_wait);
  lock_release(&count_lock);

  lock_acquire(&thread_hang);
  msg("Thread ran: %s round", str);
  lock_release(&thread_hang);
}

void test_main(void) {
  syn_msg = true;
  msg("Main started");

  // Init global data structures and acquire hang lock
  created_threads = 0;
  lock_check_init(&count_lock);
  lock_check_init(&thread_hang);
  sema_check_init(&main_wait, 0);
  lock_acquire(&thread_hang);

  // Spawn threads
  tid_t tids[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
    tids[i] = pthread_check_create(thread_function, first);

  // When main wakes, there are NUM_THREADS threads in the system
  // Release them!
  sema_down(&main_wait);
  lock_release(&thread_hang);

  // Wait on threads
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_check_join(tids[i]);

  // Main is alone again. Re-acquire the hanging lock
  created_threads = 0;
  lock_acquire(&thread_hang);

  // Spawn threads again
  for (int i = 0; i < NUM_THREADS; i++)
    tids[i] = pthread_check_create(thread_function, second);

  // When main wakes, there are NUM_THREADS threads in the system again
  // Release them!
  sema_down(&main_wait);
  lock_release(&thread_hang);

  // Wait on threads again
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_check_join(tids[i]);

  msg("Main finished");
  syn_msg = false;
}

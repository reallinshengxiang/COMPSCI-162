/* Many threads (N) are spawned, and main must wait for all of them to complete
   with only a single semaphore and no joining. Furthermore, we init a semaphore
   to N and have each thread down the semaphore at the beginning of the method
   but do NOT up to ensure that semaphores support k-mutual exclusion. */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

#define NUM_THREADS 10

// Global variables
lock_t global_lock;
int ready_threads;
sema_t main_sema;
sema_t thread_sema;

void thread_function(void* arg_);

/* Each thread downs thread_sema, grabs the global lock, increments
   ready_threads and wakes main safely */
void thread_function(void* arg_ UNUSED) {
  sema_down(&thread_sema);
  lock_acquire(&global_lock);
  msg("%d threads are now ready", ++ready_threads);
  if (ready_threads == NUM_THREADS)
    sema_up(&main_sema);
  lock_release(&global_lock);
}

void test_main(void) {
  syn_msg = true;
  msg("Main starting");

  // Initialize global data
  lock_check_init(&global_lock);
  sema_check_init(&main_sema, 0);
  sema_check_init(&thread_sema, NUM_THREADS);
  ready_threads = 0;

  // Spawn threads
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_check_create(thread_function, NULL);

  // Wait on threads
  sema_down(&main_sema);

  msg("Main finishing");
  syn_msg = false;
}

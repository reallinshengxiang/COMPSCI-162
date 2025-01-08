/* Create 128 locks and 128 semaphores, and acquire them all. Then,
   let another thread acquire them. */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

/* These locks plus the console lock add up to 128 in total */
#define NUM_SYNCH 127

// Global variables
sema_t thread_wait;
sema_t semaphores[NUM_SYNCH];
lock_t locks[NUM_SYNCH];

void thread_function(void* arg_);

/* Acquires all the locks and the semaphores */
void thread_function(void* arg_ UNUSED) {
  // Informs main that we've started
  msg("Thread started");
  sema_up(&thread_wait);

  // Acquire all the locks and semas
  for (int i = 0; i < NUM_SYNCH; i++) {
    sema_down(&semaphores[i]);
    lock_acquire(&locks[i]);
  }
  msg("Thread acquired all locks and downed semaphores");

  // Release them all
  for (int i = 0; i < NUM_SYNCH; i++) {
    sema_up(&semaphores[i]);
    lock_release(&locks[i]);
  }
  msg("Thread finished");
}

void test_main(void) {
  msg("Main starting");

  // Initialize global data
  sema_check_init(&thread_wait, 0);
  for (int i = 0; i < NUM_SYNCH; i++) {
    lock_check_init(&locks[i]);
    sema_check_init(&semaphores[i], 1);
  }

  // Acquire all the locks and down all the semaphores
  for (int i = 0; i < NUM_SYNCH; i++) {
    lock_acquire(&locks[i]);
    sema_down(&semaphores[i]);
  }
  msg("Main acquired locks and downed semaphores");

  // Create a helper thread and wait for it to start
  tid_t child_tid = pthread_check_create(thread_function, NULL);
  sema_down(&thread_wait);

  // Now release everything and let the thread acquire it
  msg("Main releasing all locks and semaphores");
  for (int i = 0; i < NUM_SYNCH; i++) {
    lock_release(&locks[i]);
    sema_up(&semaphores[i]);
  }

  pthread_check_join(child_tid);
  msg("Main finishing");
}

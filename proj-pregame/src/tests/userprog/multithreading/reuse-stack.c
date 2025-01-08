/* Creates two threads, who inform main of their stack pointers.
   Creates another thread, whose stack pointer should be at least
   as high as those threads. */

#include "tests/lib.h"
#include "tests/main.h"
#include <pthread.h>
#include <stdint.h>

#define NUM_THREADS 2

// Global variables
lock_t esp_lock;
uint32_t lowest_esp;

void get_esp_address(void* arg_);
void assert_higher_esp(void* arg_);

/* Updates lowest_esp if this thread's ESP is lower than the lowest ESP */
void get_esp_address(void* arg_) {
  lock_acquire(&esp_lock);
  uint32_t esp = (uint32_t)&arg_;
  lowest_esp = esp < lowest_esp ? esp : lowest_esp;
  lock_release(&esp_lock);
}

/* Fails if this thread's esp is lower than lowest_esp */
void assert_higher_esp(void* arg_) {
  lock_acquire(&esp_lock);
  uint32_t esp = (uint32_t)&arg_;
  if (esp < lowest_esp)
    fail("Should reuse space of exited threads");
  lock_release(&esp_lock);
}

void test_main(void) {
  msg("Main started");

  // Initialize global variables
  lock_check_init(&esp_lock);
  lowest_esp = INT32_MAX;

  // Spawn threads who set lowest_esp
  tid_t tids[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
    tids[i] = pthread_check_create(get_esp_address, NULL);

  // Wait for threads to finish
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_check_join(tids[i]);
  msg("Threads done");

  // Spawn another thread, and make sure it's stack is higher
  pthread_check_join(pthread_check_create(assert_higher_esp, NULL));
  msg("Main finished");
}

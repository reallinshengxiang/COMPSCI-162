/* Thread i spawns thread i + 1, waits for thread i + 1 to finish,
   and then continues. Waiting is managed by semaphores  */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

#define NUM_THREADS 10

// Data to pass to each thread
struct thread_data {
  int thread_num;
  sema_t sema;
};

void thread_function(void* arg_);

/* Each thread prints its thread number n, spawns a new thread with
   thread number n + 1, and waits for that thread to finish executing
   before this thread prints its thread number again and finishes */
void thread_function(void* arg_) {
  struct thread_data* my_thread_data = (struct thread_data*)arg_;
  msg("Thread %d starting", my_thread_data->thread_num);

  // Create child thread, and sleep until it finishes
  if (my_thread_data->thread_num < NUM_THREADS) {
    struct thread_data child_thread_data;
    child_thread_data.thread_num = my_thread_data->thread_num + 1;
    sema_check_init(&child_thread_data.sema, 0);
    pthread_check_create(thread_function, &child_thread_data);
    sema_down(&child_thread_data.sema);
  }

  // Child has finished, print that we finished then wake our parent
  msg("Thread %d finishing", my_thread_data->thread_num);
  sema_up(&my_thread_data->sema);
}

void test_main(void) {
  syn_msg = true;
  msg("Main starting");

  // Spawn the first thread and wait for it
  struct thread_data first_thread_data;
  first_thread_data.thread_num = 1;
  sema_check_init(&first_thread_data.sema, 0);
  pthread_check_join(pthread_check_create(thread_function, &first_thread_data));

  msg("Main finishing");
  syn_msg = false;
}

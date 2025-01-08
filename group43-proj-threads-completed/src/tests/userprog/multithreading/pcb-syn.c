/* Spawn many threads which continuously exercise the PCB. Some threads
   will continuously open and close files, other threads will continuously
   init locks and semaphores */

#include "tests/userprog/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

#define RESOURCE_MAX 126
#define SYN_MAX 18
#define NUM_IO_THREADS 100
#define NUM_SYN_THREADS 7

// Global variables
lock_t lock_store[RESOURCE_MAX];
sema_t sema_store[RESOURCE_MAX];

void file_io(void* arg_);
void syn_init(void* arg_);

/* File IO threads continuosly open and close files */
void file_io(void* arg_ UNUSED) {
  for (int i = 0; i < RESOURCE_MAX; i++)
    check_file("sample.txt", sample, sizeof sample - 1);
}

/* Syn Init threads continuously initialize new locks and semaphores.
   These locks and semas are also acquired to ensure initialization succeeded */
void syn_init(void* arg_) {
  int* thread_num = (int*)arg_;
  for (int i = 0; i < SYN_MAX; i++) {
    lock_t* lock = &lock_store[i + (*thread_num * SYN_MAX)];
    sema_t* sema = &sema_store[i + (*thread_num * SYN_MAX)];
    lock_check_init(lock);
    lock_acquire(lock);
    lock_release(lock);
    sema_check_init(sema, 1);
    sema_down(sema);
    sema_up(sema);
  }
}

void test_main(void) {
  syn_msg = true;
  msg("Main starting");
  quiet = true;

  // Spawn IO threads
  tid_t io_tids[NUM_IO_THREADS];
  for (int i = 0; i < NUM_IO_THREADS; i++)
    io_tids[i] = pthread_check_create(file_io, NULL);

  // Spawn syn threads
  int syn_data[NUM_SYN_THREADS];
  tid_t syn_tids[NUM_SYN_THREADS];
  for (int i = 0; i < NUM_SYN_THREADS; i++) {
    syn_data[i] = i;
    syn_tids[i] = pthread_check_create(syn_init, &syn_data[i]);
  }

  // Wait on all threads
  for (int i = 0; i < NUM_IO_THREADS; i++)
    pthread_check_join(io_tids[i]);
  for (int i = 0; i < NUM_SYN_THREADS; i++)
    pthread_check_join(syn_tids[i]);

  // If we got here, we were successful
  quiet = false;
  msg("Main finished");
  syn_msg = false;
}

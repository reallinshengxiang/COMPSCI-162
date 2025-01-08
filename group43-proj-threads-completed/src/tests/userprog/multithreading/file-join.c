/* Tests the fork-join model with threads that read a file.
   All the file reads should fail because the threads share the same FD
   and main has already read the file fully.
   Phase I: open a file
   Phase II: fork threads that read the file from the same fd
   Phase III: join on those threads
   Phase IV: fork threads that read the file from the same fd
   Phase V: join on the threads using pthread_exit on the main thread */

#include "tests/userprog/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

#define FILE_CHECKS 126
#define NUM_THREADS 10

// Global variables
char file_storage[512]; // Large enough to fit sample.txt
char* sample_file = "sample.txt";
lock_t count_lock;
int num_finished;

void file_io(void* arg_);

/* Argument is an FD. Read that file multiple times */
void file_io(void* arg_) {
  int* fd = (int*)arg_;
  for (int i = 0; i < FILE_CHECKS; i++)
    if (read(*fd, &file_storage, sizeof sample - 1) > 0)
      fail("Read bytes from used-up FD %d", *fd);

  lock_acquire(&count_lock);
  if (++num_finished == NUM_THREADS * 2) {
    quiet = false;
    msg("All threads complete");
    syn_msg = false;
  }
  lock_release(&count_lock);
}

void test_main(void) {
  syn_msg = true;
  msg("Main starting");
  quiet = true;

  // Initialize global variables
  int fd = open(sample_file);
  ASSERT(fd > 1);
  check_file_handle(fd, sample_file, sample, sizeof sample - 1);
  lock_check_init(&count_lock);
  num_finished = 0;

  // Spawn IO threads
  tid_t tids[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
    tids[i] = pthread_check_create(file_io, &fd);

  // Wait on all threads
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_check_join(tids[i]);

  // Spawn IO threads again
  for (int i = 0; i < NUM_THREADS; i++)
    tids[i] = pthread_check_create(file_io, &fd);

  // This should join on all the threads
  pthread_exit();
  NOT_REACHED();
}

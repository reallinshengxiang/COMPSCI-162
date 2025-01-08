/* Spawns a new process that spawns many child threads that each
   access file resources. Then, kills the whole process via exit/exception.
   Then, the original process tries to access file resources itself to make
   sure resources were cleanly handled on child process exit */

#include "tests/userprog/sample.inc"
#include "tests/lib.h"
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <pthread.h>

#define NUM_THREADS 100
#define NUM_ACCESS 100

// Global variables
char* sample_file = "sample.txt";

void thread_function(void* arg_);

/* Does a bunch of IO */
void thread_function(void* arg_ UNUSED) {
  for (int i = 0; i < NUM_ACCESS; i++)
    check_file(sample_file, sample, sizeof sample - 1);
}

int main(int argc, char* argv[]) {
  test_name = "exit-clean";

  if (argc == 4 && !strcmp(argv[1], "root")) {
    // Spawn a new process that tries to create NUM_THREADS threads
    msg("Root process starting");
    char child_cmd[128];
    snprintf(child_cmd, sizeof child_cmd, "%s-%s thread %s", test_name, argv[3], argv[2]);
    wait(exec(child_cmd));
    msg("Root process continuing");

    // Now access file resources
    quiet = true;
    for (int i = 0; i < NUM_ACCESS; i++)
      check_file(sample_file, sample, sizeof sample - 1);
    quiet = false;

    msg("Success!");
  } else if (argc == 3 && !strcmp(argv[1], "thread")) {
    msg("Spawning child threads");
    quiet = true;

    // Spawn some threads and then join them
    for (int i = 0; i < NUM_THREADS; i++)
      pthread_check_create(thread_function, NULL);

    // Waste some time accessing the file system
    check_file(sample_file, sample, sizeof sample - 1);

    // Kill the process via exit call or exception
    bool exit_syscall = !strcmp(argv[2], "exit");
    if (exit_syscall) {
      exit(162);
    } else {
      int null_val = *(volatile int*)NULL;
      msg("I dereferenced null! It's value is %d", null_val);
    }

    quiet = false;
    fail("Somehow we didn't successfully kill the process");
  } else
    fail("Incorrect usage");

  return 0;
}

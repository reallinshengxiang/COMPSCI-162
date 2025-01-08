/* The root process exec's the same file with a different argument,
   which instructs the child to create some threads.
   The root process waits on the child process to have the created threads
   run and then finishes itself. */

#include "tests/lib.h"
#include <string.h>
#include <syscall.h>
#include <pthread.h>

#define NUM_THREADS 10

void thread_function(void* arg_);

/* Simple thread function */
void thread_function(void* arg_ UNUSED) { msg("Child thread ran"); }

int main(int argc, char* argv[]) {
  test_name = "exec-thread-n";

  if (argc != 2)
    fail("Incorrect usage");

  if (!strcmp(argv[1], "root")) {
    // Spawn a new process that tries to create NUM_THREADS threads
    msg("Root process starting");
    wait(exec("exec-thread-n thread"));
    msg("Root process finishing");
  } else if (!strcmp(argv[1], "thread")) {
    // Spawn some threads and then join them
    msg("Spawning child threads");
    tid_t tids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
      tids[i] = pthread_check_create(thread_function, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
      pthread_check_join(tids[i]);
    msg("Finished waiting on children");
  } else
    fail("Incorrect usage");
}

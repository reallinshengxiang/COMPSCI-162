/* Ensure that only one thread can call wait on a subprocess */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

// Thread data
struct thread_data {
  pid_t child_pid;
  bool should_succeed;
};

void thread_function(void* arg_);

/* Waits on a child process */
void thread_function(void* arg_) {
  struct thread_data* thread_data = (struct thread_data*)arg_;
  pid_t return_value = wait(thread_data->child_pid);
  if (thread_data->should_succeed) {
    ASSERT(return_value == 81);
  } else {
    ASSERT(return_value == -1);
  }
  msg("Thread successfully finished waiting");
}

void test_main(void) {
  // Exec a child process
  pid_t child_pid = exec("child-simple");

  // Setup data for threads
  struct thread_data success, failure;
  success.should_succeed = true;
  success.child_pid = child_pid;
  failure.should_succeed = false;
  failure.child_pid = child_pid;

  // Spawn success thread and join
  pthread_check_join(pthread_check_create(thread_function, &success));

  // Spawn failure thread and join
  pthread_check_join(pthread_check_create(thread_function, &failure));
}

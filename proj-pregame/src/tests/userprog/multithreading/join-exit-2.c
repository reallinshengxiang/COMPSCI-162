/* Main spanws a thread that joins on main.
   Then, main pthread_exit()s and wakes the joiner.
   pthread_exit() on main waits for all unjoined threads.
   So main should wait for the created thread.
   The created thread then runs and calls exit(162), which
   should terminate the process gracefully with exit code 162 */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

void thread_function(void* arg_);

/* Thread function that tests exit conditions by exiting */
void thread_function(void* arg_) {
  int* main_tid = (int*)arg_;
  msg("Thread starting");
  pthread_check_join(*main_tid);
  exit(162);
  fail("Should not be here");
}

void test_main(void) {
  msg("Main starting");
  tid_t main_tid = get_tid();
  pthread_check_create(thread_function, &main_tid);
  pthread_exit();
  fail("Should not be here");
}

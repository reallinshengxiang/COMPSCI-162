/* Main spanws a thread that joins on main.
   Then, main pthread_exit()s and wakes the joiner.
   pthread_exit() on main waits for all unjoined threads.
   So main should wait for the created thread to finish executing.
   Then, main should terminate the process gracefully with exit code 0 */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

void thread_function(void* arg_);

/* Thread function that tests exit conditions */
void thread_function(void* arg_) {
  int* main_tid = (int*)arg_;
  msg("Thread starting");
  pthread_check_join(*main_tid);
  msg("Thread finished");
}

void test_main(void) {
  msg("Main starting");
  tid_t main_tid = get_tid();
  pthread_check_create(thread_function, &main_tid);
  pthread_exit();
  fail("Should not be here");
}

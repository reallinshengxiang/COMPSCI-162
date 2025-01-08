/* The root process spawns a thread and waits until it starts running.
   The thread waits on root main, which exec's another process.
   We want to test that the exec'd process only has one main thread of
   control, and not a "copy" of the child thread created above.
   We do this by having the exec'd process pthread_exit() to wait on
   all other threads. Nothing should be printed except the exit code.
   Then, both root and the child thread finish when root main calls pthread_exit() */

#include "tests/lib.h"
#include <string.h>
#include <syscall.h>
#include <pthread.h>

// Global variables
sema_t hang_main;

void thread_function(void* arg_);

/* Informs main it started, waits until main finishes. */
void thread_function(void* arg_) {
  tid_t* main_tid = (tid_t*)arg_;
  msg("Child thread starting");
  sema_up(&hang_main);
  pthread_check_join(*main_tid);
  msg("Child thread finishing");
}

int main(int argc, char* argv[]) {
  test_name = "exec-thread-1";

  if (argc != 2)
    fail("Incorrect usage");

  if (!strcmp(argv[1], "exit"))
    // Leave, and make sure we were the only thread
    pthread_exit();
  else if (!strcmp(argv[1], "thread")) {
    // Spawn a thread that waits on main to finish
    msg("Creating child thread");
    sema_check_init(&hang_main, 0);
    tid_t main_tid = get_tid();
    pthread_check_create(thread_function, &main_tid);
    sema_down(&hang_main);

    // Exec a child process that exits
    wait(exec("exec-thread-1 exit"));

    // Wait for thread to finish
    pthread_exit();
  } else
    fail("Incorrect usage");
}

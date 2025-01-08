#include <pthread.h>
#include <syscall.h>

void _pthread_start_stub(pthread_fun fun, void* arg);

/* Creates a new thread running fun with the given arg.
   Calls pthread_exit when the function completes.
   Returns TID of created thread or TID_ERROR on error */
tid_t pthread_create(pthread_fun fun, void* arg) {
  return sys_pthread_create(_pthread_start_stub, fun, arg);
}

/* Exits the current thread, and cleans up resources */
void pthread_exit() {
  sys_pthread_exit();
  NOT_REACHED();
}

/* Waits for thread TID to finish executing before returning.
   Returns false if an error occurred. */
bool pthread_join(tid_t tid) { return sys_pthread_join(tid) != TID_ERROR; }

/* OS jumps to this function when a new thread is created.
   OS is required to setup the stack for this function and
   set %eip to point to the start of this function */
void _pthread_start_stub(pthread_fun fun, void* arg) {
  (*fun)(arg);    // Invoke the thread function
  pthread_exit(); // Call pthread_exit
}

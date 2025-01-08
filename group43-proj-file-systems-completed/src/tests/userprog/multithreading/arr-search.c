/* Create threads that search for elements in an array */

#include "tests/lib.h"
#include "tests/main.h"
#include <pthread.h>
#include <syscall.h>

#define ARR_SIZE (1 << 5)

// Global variables
int arr[ARR_SIZE];

// Thread data
struct thread_data {
  // Input arguments
  int start_idx; // Inclusive
  int end_idx;   // Exclusive
  int elem;      // Elem to find

  // Output arguments
  bool elem_found;
  int elem_idx;
};

void thread_function(void* arg_);
void search_and_check(int elem);

/* Searches for elem in range [start_idx, end_idx) by
   spawning helper threads */
void thread_function(void* arg_) {
  struct thread_data* thread_data = (struct thread_data*)arg_;

  // Base case, we only have one element to search
  if (thread_data->end_idx - thread_data->start_idx == 1) {
    thread_data->elem_found = (arr[thread_data->start_idx] == thread_data->elem);
    thread_data->elem_idx = thread_data->start_idx;
    return;
  }

  // Recursive case, we split into two threads
  struct thread_data left, right;
  int midpoint = (thread_data->start_idx + thread_data->end_idx) / 2;
  left.start_idx = thread_data->start_idx;
  left.end_idx = midpoint;
  left.elem = thread_data->elem;
  right.start_idx = midpoint;
  right.end_idx = thread_data->end_idx;
  right.elem = thread_data->elem;

  tid_t left_tid = pthread_check_create(thread_function, &left);
  tid_t right_tid = pthread_check_create(thread_function, &right);
  pthread_check_join(left_tid);
  pthread_check_join(right_tid);

  // Return if either one of our threads found the answer
  if (left.elem_found) {
    thread_data->elem_found = true;
    thread_data->elem_idx = left.elem_idx;
  } else if (right.elem_found) {
    thread_data->elem_found = true;
    thread_data->elem_idx = right.elem_idx;
  } else {
    thread_data->elem_found = false;
  }
}

/* Searches for elem in array via thread_function, and
   checks that the answer is as expected */
void search_and_check(int elem) {
  // Generate the expected answers from what we know about arr
  int expected_found = elem >= 0 && elem < ARR_SIZE;
  int expected_idx = ARR_SIZE - elem - 1;

  // Spawn a thread to find the elem in arr
  struct thread_data thread_data;
  thread_data.start_idx = 0;
  thread_data.end_idx = ARR_SIZE;
  thread_data.elem = elem;
  pthread_check_join(pthread_check_create(thread_function, &thread_data));

  // Check that the thread's answer is correct
  if (expected_found) {
    ASSERT(thread_data.elem_found);
    ASSERT(thread_data.elem_idx == expected_idx);
  } else {
    ASSERT(!thread_data.elem_found);
  }
}

void test_main(void) {
  msg("Main starting");

  // Initialize global variables
  for (int i = 0; i < ARR_SIZE; i++)
    arr[i] = ARR_SIZE - i - 1;

  // Spawn threads to search for elements in the array
  for (int i = 0; i < ARR_SIZE; i++)
    search_and_check(i);

  // Spawn threads to search for elements not in the array
  for (int i = -ARR_SIZE; i < 0; i++)
    search_and_check(i);

  for (int i = ARR_SIZE; i < ARR_SIZE * 2; i++)
    search_and_check(i);

  msg("Main finished");
}

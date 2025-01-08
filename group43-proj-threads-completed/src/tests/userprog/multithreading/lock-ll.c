/* Spawns two threads each trying to add to a linked-list */

#include "tests/lib.h"
#include "tests/main.h"
#include <syscall.h>
#include <pthread.h>

#define NUM_THREADS 11
#define SUM_TO 100

// Singly-linked list node
struct node {
  int val;
  struct node* rest;
};

// To pass into each thread
struct thread_data {
  int thread_num;
  lock_t* ll_lock;
  struct node* head_ptr;
  struct node* node_store;
};

// Global variables
lock_t lock;
struct node head;
struct node node_store[NUM_THREADS * SUM_TO];

void thread_function(void* arg_);
struct node* list_tail(struct node* head);
int list_sum(struct node* head);

/* Gets the tail of a SLL from its head. Assumes head is non-null */
struct node* list_tail(struct node* head) {
  struct node* cur = head;
  for (; cur->rest != NULL; cur = cur->rest)
    ;
  return cur;
}

/* Sums the values of the linked list. */
int list_sum(struct node* head) {
  int sum = 0;
  for (struct node* cur = head; cur != NULL; cur = cur->rest)
    sum += cur->val;
  return sum;
}

/* Scans to the back of the list, and appends a new value */
void thread_function(void* arg_) {
  struct thread_data* thread_data = (struct thread_data*)arg_;
  for (int i = 0; i < SUM_TO; i++) {
    lock_acquire(thread_data->ll_lock);
    struct node* tail = list_tail(thread_data->head_ptr);
    struct node* new_node = thread_data->node_store + (i + SUM_TO * thread_data->thread_num);
    new_node->val = i + 1;
    new_node->rest = NULL;
    tail->rest = new_node;
    lock_release(thread_data->ll_lock);
  }
}

void test_main(void) {
  lock_check_init(&lock);

  // Initialize head. Lock acquire here isn't necessary, but oh well.
  lock_acquire(&lock);
  head.val = 0;
  head.rest = NULL;
  lock_release(&lock);

  // Setup thread data
  struct thread_data thread_data[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_data[i].thread_num = i;
    thread_data[i].ll_lock = &lock;
    thread_data[i].head_ptr = &head;
    thread_data[i].node_store = node_store;
  }

  // Spawn threads
  tid_t tids[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
    tids[i] = pthread_check_create(thread_function, &thread_data[i]);

  // Wait on threads
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_check_join(tids[i]);

  // Check that the value is correct
  lock_acquire(&lock);
  int final = list_sum(&head);
  lock_release(&lock);

  // Compute the answer and verify
  int correct_answer = 0;
  for (int i = 1; i <= SUM_TO; i++)
    correct_answer += i;
  correct_answer *= NUM_THREADS;

  if (final != correct_answer)
    fail("%d is the answer, but you thought it was %d", correct_answer, final);
  msg("PASS");
}

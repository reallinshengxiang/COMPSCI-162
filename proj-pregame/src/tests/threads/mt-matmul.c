/* Simple implementation of a C = C + AxB matrix multiplication,
   where A, B, and C are NxN matrices. */

/* Based on UC Berkeley's RISC-V benchmark of the same name:
   https://github.com/ucb-bar/riscv-benchmarks/tree/master/mt-matmul */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "tests/threads/matmul_data.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

void __attribute__((noinline)) matmul(const int tid, const int nthreads, const int lda,
                                      const short A[], const short B[], short C[]);
void test_mt_matmul(size_t num_threads);
void thread_entry(void* aux);

static short results_data[ARRAY_SIZE];

struct thread_args {
  int tid;
  int n_threads;
};

void __attribute__((noinline)) matmul(const int tid, const int nthreads, const int lda,
                                      const short A[], const short B[], short C[]) {
  int i, j, k;
  int block = lda / nthreads;
  int start = block * tid;

  for (j = start; j < (start + block); j++)
    for (k = 0; k < lda; k++)
      for (i = 0; i < lda; i++)
        C[i + j * lda] += A[j * lda + k] * B[k * lda + i];
}

void thread_entry(void* aux) {
  struct thread_args* args = (struct thread_args*)aux;

  matmul(args->tid, args->n_threads, DIM_SIZE, input1_data, input2_data, results_data);
}

void test_mt_matmul(size_t num_threads) {
  ASSERT(active_sched_policy == SCHED_PRIO);

  /* Make sure our priority is the default. */
  ASSERT(thread_get_priority() == PRI_DEFAULT);

  struct thread_args args[num_threads];
  for (size_t i = 0; i < num_threads; i++) {
    args[i].tid = i;
    args[i].n_threads = num_threads;

    thread_create("matmul", PRI_DEFAULT - 1, thread_entry, (void*)&args[i]);
  }

  /* Let other threads run to completion */
  thread_set_priority(PRI_DEFAULT - 2);

  int res = verifyDouble(ARRAY_SIZE, results_data, verify_data);

  if (res) {
    msg("Matrix results match expected values.");
  } else {
    msg("Matrix results do not match expected values!");
  }
}

void test_mt_matmul_1(void) {
  msg("Executing single-threaded matmul...");
  test_mt_matmul(1);
}
void test_mt_matmul_2(void) {
  msg("Executing blocked matmul with 2 threads...");
  test_mt_matmul(2);
}
void test_mt_matmul_4(void) {
  msg("Executing blocked matmul with 4 threads...");
  test_mt_matmul(4);
}
void test_mt_matmul_16(void) {
  msg("Executing blocked matmul with 16 threads...");
  test_mt_matmul(16);
}

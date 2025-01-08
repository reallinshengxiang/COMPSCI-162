#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include <list.h>
#include <stdint.h>
#include "threads/synch.h"

// At most 8MB can be allocated to the stack
// These defines will be used in Project 2: Multithreading
#define MAX_STACK_PAGES (1 << 11)
#define MAX_ARGS 1024
#define MAX_THREADS 127

/* PIDs and TIDs are the same type. PID should be
   the TID of the main thread of the process */
typedef tid_t pid_t;

/* Thread functions (Project 2: Multithreading) */
typedef void (*pthread_fun)(void*);
typedef void (*stub_fun)(pthread_fun, void*);

/* The process control block for a given process. Since
   there can be multiple threads per process, we need a separate
   PCB from the TCB. All TCBs in a process will have a pointer
   to the PCB, and the PCB will have a pointer to the main thread
   of the process, which is `special`. */
struct process {
  /* Owned by process.c. */
  struct wait_status* wait_status; /* This process's completion status. */
  struct list children;            /* Completion status of children. */
  uint32_t* pagedir;               /* Page directory. */
  char process_name[16];           /* Name of the main thread */
  struct file* bin_file;           /* Executable. */
  struct thread* main_thread;      /* Pointer to main thread */

  /* Owned by syscall.c. */
  struct list fds; /* List of file descriptors. */
  int next_handle; /* Next handle value. */
};

/* Tracks the completion of a process.
   Reference held by both the parent, in its `children' list,
   and by the child, in its `wait_status' pointer. */
struct wait_status {
  struct list_elem elem; /* `children' list element. */
  struct lock lock;      /* Protects ref_cnt. */
  int ref_cnt;           /* 2=child and parent both alive,
                                           1=either child or parent alive,
                                           0=child and parent both dead. */
  pid_t pid;             /* Child process id. */
  int exit_code;         /* Child exit code, if dead. */
  struct semaphore dead; /* 1=child alive, 0=child dead. */
};

/* A file descriptor, for binding a file handle to a file. */
struct file_descriptor {
  struct list_elem elem; /* List element. */
  struct file* file;     /* File. */
  int handle;            /* File handle. */
};

void userprog_init(void);

pid_t process_execute(const char* file_name);
int process_wait(pid_t);
void process_exit(void);
void process_activate(void);

bool is_main_thread(struct thread*, struct process*);
pid_t get_pid(struct process*);

tid_t pthread_execute(stub_fun, pthread_fun, void*);
tid_t pthread_join(tid_t);
void pthread_exit(void);
void pthread_exit_main(void);

#endif /* userprog/process.h */

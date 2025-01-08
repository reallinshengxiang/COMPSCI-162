#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include <stdint.h>

// At most 8MB can be allocated to the stack
// These defines will be used in Project 2: Multithreading
#define MAX_STACK_PAGES (1 << 11)
#define MAX_THREADS 127

#define MAX_FILES 128

typedef struct file_descriptor {
  struct file* file;
  struct list_elem elem;
} file_descriptor;

typedef struct file_descriptor_list {
  int next_fd;
  struct list lst;
} file_descriptor_list;

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
struct loader {
  struct semaphore load_sema;
  bool success;
};

struct child_process {
  pid_t pid;                  /* Process ID */
  int running;                /* 1 if running, 0 if killed */
  int exit_id;                /* -1 if not exited, exit ID otherwise */
  int waited;                 /* 1 if process_wait(pid) has been called */
  int refcount;               /* Holds reference count to child processs struct*/
  struct semaphore done_sema; /* Semaphore signals when parent can execute */
  struct list_elem elem;
  struct lock child_lock;
};

struct process {
  /* Owned by process.c. */
  uint32_t* pagedir; /* Page directory. */
  struct file* executable;
  struct file_descriptor fd;
  struct file_descriptor_list fd_list;
  char process_name[16];      /* Name of the main thread */
  struct thread* main_thread; /* Pointer to main thread */

  struct process* parent; /* Pointer to parent PCB */
  struct list children;   /* List of child_process structs */
  struct loader*
      loader; /* Struct containing load semaphore and success info; shared between parent & child*/
};

struct process_initializer {
  struct process* parent;
  void* fn_copy;
  struct file* executable;
};

void userprog_init(void);

pid_t process_execute(const char* file_name);
int process_wait(pid_t);
void process_exit(int exit_id);
void process_activate(void);

bool is_main_thread(struct thread*, struct process*);
pid_t get_pid(struct process*);

tid_t pthread_execute(stub_fun, pthread_fun, void*);
tid_t pthread_join(tid_t);
void pthread_exit(void);
void pthread_exit_main(void);

#endif /* userprog/process.h */

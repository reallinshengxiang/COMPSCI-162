#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/thread.h"
#include "userprog/process.h"

int sys_halt(void);
int sys_exit(int status);
int sys_exec(const char* ufile);
int sys_wait(pid_t child);
int sys_create(const char* ufile, unsigned initial_size);
int sys_remove(const char* ufile);
int sys_open(const char* ufile);
int sys_filesize(int handle);
int sys_read(int handle, void* udst_, unsigned size);
int sys_write(int handle, void* usrc_, unsigned size);
int sys_seek(int handle, unsigned position);
int sys_tell(int handle);
int sys_close(int handle);
int sys_practice(int input);
int sys_compute_e(int n);
tid_t sys_pthread_create(stub_fun sfun, pthread_fun tfun, const void* arg);
void sys_pthread_exit(void) NO_RETURN;
tid_t sys_pthread_join(tid_t tid);
bool sys_lock_init(lock_t* lock);
int sys_lock_acquire(lock_t* lock);
int sys_lock_release(lock_t* lock);
bool sys_sema_init(sema_t* sema, int val);
bool sys_sema_down(sema_t* sema);
bool sys_sema_up(sema_t* sema);

bool sys_sema_init(sema_t* sema, int val);
bool sys_sema_up(sema_t* sema);
bool sys_sema_down(sema_t* sema);
tid_t sys_get_tid(void);

void syscall_init(void);
void safe_file_close(struct file* file);

#endif /* userprog/syscall.h */

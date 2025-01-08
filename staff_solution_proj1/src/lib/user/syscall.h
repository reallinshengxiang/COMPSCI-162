#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H

#include <stdbool.h>
#include <debug.h>
#include <pthread.h>

/* Process identifier. */
typedef int pid_t;
#define PID_ERROR ((pid_t)-1)

/* Synchronization Types */
typedef char lock_t;
typedef char sema_t;

/* Map region identifier. */
typedef int mapid_t;
#define MAP_FAILED ((mapid_t)-1)

/* Maximum characters in a filename written by readdir(). */
#define READDIR_MAX_LEN 14

/* Typical return values from main() and arguments to exit(). */
#define EXIT_SUCCESS 0 /* Successful execution. */
#define EXIT_FAILURE 1 /* Unsuccessful execution. */

/* Projects 2 and later. */
void halt(void) NO_RETURN;
void exit(int status) NO_RETURN;
pid_t exec(const char* file);
int wait(pid_t);
bool create(const char* file, unsigned initial_size);
bool remove(const char* file);
int open(const char* file);
int filesize(int fd);
int read(int fd, void* buffer, unsigned length);
int write(int fd, const void* buffer, unsigned length);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
int practice(int i);
double compute_e(int n);
tid_t sys_pthread_create(stub_fun sfun, pthread_fun tfun, const void* arg);
void sys_pthread_exit(void) NO_RETURN;
tid_t sys_pthread_join(tid_t tid);
bool lock_init(lock_t* lock);
void lock_acquire(lock_t* lock);
void lock_release(lock_t* lock);
bool sema_init(sema_t* sema, int val);
void sema_down(sema_t* sema);
void sema_up(sema_t* sema);
tid_t get_tid(void);

/* Project 3 and optionally project 4. */
mapid_t mmap(int fd, void* addr);
void munmap(mapid_t);

/* Project 4 only. */
bool chdir(const char* dir);
bool mkdir(const char* dir);
bool readdir(int fd, char name[READDIR_MAX_LEN + 1]);
bool isdir(int fd);
int inumber(int fd);

#endif /* lib/user/syscall.h */

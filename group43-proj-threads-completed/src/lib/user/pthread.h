#ifndef __LIB_USER_PTHREAD_H
#define __LIB_USER_PTHREAD_H

#include <debug.h>
#include <stdbool.h>

/* Thread identifiers and thread function */
typedef void (*pthread_fun)(void*);
typedef void (*stub_fun)(pthread_fun, void*);
typedef int tid_t;
#define TID_ERROR ((tid_t)-1)

tid_t pthread_create(pthread_fun fun, void* arg);
void pthread_exit(void) NO_RETURN;
bool pthread_join(tid_t);

#endif /* lib/user/pthread.h */

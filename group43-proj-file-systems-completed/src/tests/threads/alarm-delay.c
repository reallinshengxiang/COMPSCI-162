#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

static thread_func alarm_delay_thread;
static int64_t delay_time[10];
static struct semaphore wait_sema;

void test_alarm_delay(void) {
  int i;
  sema_init(&wait_sema, 0);
  for (i = 0; i < 10; i++) {
    delay_time[i] = (i + 1) * TIMER_FREQ;
    char name[16];
    snprintf(name, sizeof name, "delay %d", i);
    thread_create(name, PRI_DEFAULT, alarm_delay_thread, &delay_time[i]);
  }
  for (i = 0; i < 10; i++) {
    sema_down(&wait_sema);
  }
}

static void alarm_delay_thread(void* aux) {
  int64_t* delay_time_ptr = aux;
  int64_t start_time = timer_ticks();
  timer_sleep(*delay_time_ptr);
  int64_t wake_time = timer_ticks();
  int64_t actual_delay = wake_time - start_time;
  int64_t expected_delay = *delay_time_ptr;
  msg("Thread with delay %lld (expected delay was %lld, actual delay was %lld).", *delay_time_ptr,
      expected_delay, actual_delay);
  sema_up(&wait_sema);
}

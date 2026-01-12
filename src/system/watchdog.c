#include "pufu/watchdog.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static pthread_t watchdog_thread;
static int watchdog_active = 0;
static int watchdog_timeout = 0;
static void (*current_rollback)(void) = NULL;
static pthread_mutex_t watchdog_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t watchdog_cond = PTHREAD_COND_INITIALIZER;
static int stop_thread = 0;

// Thread loop
static void *watchdog_loop(void *arg) {
  (void)arg;
  while (1) {
    pthread_mutex_lock(&watchdog_mutex);

    while (!watchdog_active && !stop_thread) {
      pthread_cond_wait(&watchdog_cond, &watchdog_mutex);
    }

    if (stop_thread) {
      pthread_mutex_unlock(&watchdog_mutex);
      break;
    }

    int ms = watchdog_timeout;
    pthread_mutex_unlock(&watchdog_mutex);

    // Sleep in chunks or use condition timedwait for better responsiveness?
    // Simple usleep for now, but strictly this blocks cancellation somewhat.
    // Better implementation: use pthread_cond_timedwait in the loop above.
    // But for simplicity of logic logic 'arm' wakes us up.

    // Wait for timeout OR disarm
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long nsec = ts.tv_nsec + (ms * 1000000L);
    ts.tv_sec += nsec / 1000000000L;
    ts.tv_nsec = nsec % 1000000000L;

    pthread_mutex_lock(&watchdog_mutex);
    if (watchdog_active) {
      int res = pthread_cond_timedwait(&watchdog_cond, &watchdog_mutex, &ts);
      if (res != 0) { // Timeout!
        if (watchdog_active) {
          printf("[Watchdog] BARK! Timeout reached (%d ms). Executing "
                 "Rollback...\n",
                 ms);
          if (current_rollback) {
            current_rollback();
          } else {
            printf("[Watchdog] FATAL: No rollback function set. Aborting.\n");
            exit(1);
          }
          watchdog_active = 0;
        }
      }
    }
    pthread_mutex_unlock(&watchdog_mutex);
  }
  return NULL;
}

void pufu_watchdog_init(void) {
  if (pthread_create(&watchdog_thread, NULL, watchdog_loop, NULL) != 0) {
    perror("Failed to create watchdog thread");
    exit(1);
  }
  printf("[System] Watchdog initialized.\n");
}

void pufu_watchdog_arm(int timeout_ms) {
  pthread_mutex_lock(&watchdog_mutex);
  watchdog_timeout = timeout_ms;
  watchdog_active = 1;
  pthread_cond_signal(&watchdog_cond); // Wake up thread
  pthread_mutex_unlock(&watchdog_mutex);
  // printf("[Watchdog] Armed (%d ms)\n", timeout_ms);
}

void pufu_watchdog_disarm(void) {
  pthread_mutex_lock(&watchdog_mutex);
  watchdog_active = 0;
  pthread_cond_signal(&watchdog_cond); // Wake up thread (to cancel wait)
  pthread_mutex_unlock(&watchdog_mutex);
  // printf("[Watchdog] Disarmed\n");
}

void pufu_watchdog_set_rollback(void (*rollback_func)(void)) {
  pthread_mutex_lock(&watchdog_mutex);
  current_rollback = rollback_func;
  pthread_mutex_unlock(&watchdog_mutex);
}

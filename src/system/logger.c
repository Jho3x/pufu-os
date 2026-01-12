#include "pufu/logger.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_LOG_LINES 128
#define MAX_LINE_LEN 256

static char ring_buffer[MAX_LOG_LINES][MAX_LINE_LEN];
static int ring_head = 0; // Insertion point
static int is_full = 0;

void pufu_logger_init(void) {
  ring_head = 0;
  is_full = 0;
  // Pre-fill? No/Optional
}

void pufu_logger_append(const char *msg) {
  // Basic timestamp?
  // time_t now = time(NULL);
  // struct tm *t = localtime(&now);
  // But format might be in msg already.
  // Let's just store the raw message for now, userspace/kernel usually adds
  // prefixes.

  strncpy(ring_buffer[ring_head], msg, MAX_LINE_LEN - 1);
  ring_buffer[ring_head][MAX_LINE_LEN - 1] = '\0'; // Ensure null term

  ring_head = (ring_head + 1) % MAX_LOG_LINES;
  if (ring_head == 0)
    is_full = 1;
}

void pufu_logger_dump(const char *filename) {
  FILE *f = fopen(filename, "w");
  if (!f)
    return;

  fprintf(f, "=== Pufu OS Crash Log ===\n");
  fprintf(f, "Dump Time: %ld\n\n", time(NULL));

  // If full, start from ring_head (oldest), go to MAX, then 0 to ring_head.
  // If not full, start from 0 to ring_head.

  int count = is_full ? MAX_LOG_LINES : ring_head;
  int start = is_full ? ring_head : 0;

  for (int i = 0; i < count; i++) {
    int idx = (start + i) % MAX_LOG_LINES;
    fprintf(f, "%s\n", ring_buffer[idx]);
  }

  fprintf(f, "\n=== End of Log ===\n");
  fclose(f);
}

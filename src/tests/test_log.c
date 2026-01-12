#include "../include/pufu/logger.h"
#include <stdio.h>

int main() {
  printf("Testing Logger...\n");
  pufu_logger_init();
  pufu_logger_append("Test Log Entry 1: Init");
  pufu_logger_append("Test Log Entry 2: Operation");
  pufu_logger_append("Test Log Entry 3: Shutdown");

  pufu_logger_dump("test_output.log");
  printf("Dump Complete.\n");
  return 0;
}

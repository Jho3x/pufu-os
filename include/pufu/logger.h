#ifndef PUFU_LOGGER_H
#define PUFU_LOGGER_H

// Core Logging System
// Captures system events in a ring buffer for crash analysis

void pufu_logger_init(void);
void pufu_logger_append(const char *msg);
void pufu_logger_dump(const char *filename);

#endif // PUFU_LOGGER_H

#ifndef PUFU_TERMINAL_H
#define PUFU_TERMINAL_H

#include <stdarg.h>

// Global buffers for display discipline
extern char active_prompt[64];
extern char active_input_buffer[256];

// Initialize terminal in RAW mode (non-blocking, no echo)
void pufu_terminal_init(void);

// Restore terminal to original settings
void pufu_terminal_restore(void);

// Non-blocking character read (returns 0 if no input)
int pufu_terminal_get_key(void);

// Log a message cleanly without breaking the input line
void pufu_log(const char *fmt, ...);

// Set the active prompt string (e.g. "User@Pufu>")
void pufu_terminal_set_prompt(const char *prompt);

// Update global input buffer (called by shell/kernel)
void pufu_terminal_update_buffer(const char *buffer);

// Clear the terminal screen (ANSI)
void pufu_terminal_clear_screen(void);

// Get the pointer to the retroactive boot log buffer
// Get the pointer to the retroactive boot log buffer
const char *pufu_get_boot_logs(void);

// Terminal Window Spaces (TWS)
#define MAX_TWS 4
#define TWS_HISTORY_SIZE 8192

typedef struct {
  int id;
  char prompt[64];
  char input_buffer[256];
  char history[TWS_HISTORY_SIZE];
  int history_pos;
} PufuTerminalSpace;

// Switch active workspace
void pufu_tws_switch(int id);

// Initialize TWS system
void pufu_tws_init(void);

// Get Active TWS ID
int pufu_tws_get_active(void);

// Log to a specific TWS (or active)
void pufu_tws_log(int tws_id, const char *fmt, ...);

#endif // PUFU_TERMINAL_H

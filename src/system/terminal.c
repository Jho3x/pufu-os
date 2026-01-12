#include "pufu/terminal.h"
#include "pufu/logger.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

// Global State
char active_prompt[64] = "";
char active_input_buffer[256] = "";
static struct termios orig_termios;
static int terminal_configured = 0;

void pufu_terminal_init(void) {
  if (terminal_configured)
    return;

  if (!isatty(STDIN_FILENO)) {
    printf("[Terminal] WARN: Not a TTY. Skipping raw mode.\n");
    terminal_configured = 1;
    return;
  }

  tcgetattr(STDIN_FILENO, &orig_termios);
  struct termios newt = orig_termios;

  // Disable CANON (line buffering) and ECHO (double typing fix)
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VMIN] = 0;
  newt.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  // Set non-blocking read
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

  terminal_configured = 1;
}

void pufu_terminal_restore(void) {
  if (!terminal_configured)
    return;

  tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);

  // Restore blocking
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);

  terminal_configured = 0;
}

int pufu_terminal_get_key(void) {
  unsigned char ch;
  if (read(STDIN_FILENO, &ch, 1) > 0) {
    return (int)ch;
  }
  return 0;
}

// TWS State
static PufuTerminalSpace workspaces[MAX_TWS];
static int active_tws_id = 0;

void pufu_tws_init(void) {
  for (int i = 0; i < MAX_TWS; i++) {
    workspaces[i].id = i;
    workspaces[i].history_pos = 0;
    memset(workspaces[i].history, 0, TWS_HISTORY_SIZE);
    strcpy(workspaces[i].prompt, "Pufu> "); // Default prompt
    strcpy(workspaces[i].input_buffer, "");
  }
  // Copy initial setup to TWS 0
  strcpy(workspaces[0].prompt, active_prompt);
  strcpy(workspaces[0].input_buffer, active_input_buffer);
}

void pufu_tws_switch(int id) {
  if (id < 0 || id >= MAX_TWS)
    return;

  // Save current state
  strcpy(workspaces[active_tws_id].prompt, active_prompt);
  // Do NOT save the current input buffer, as it contains the "tws <id>"
  // command. We want a clean prompt when we return.
  strcpy(workspaces[active_tws_id].input_buffer, "");

  // Switch ID
  active_tws_id = id;

  // Restore state
  strcpy(active_prompt, workspaces[active_tws_id].prompt);
  strcpy(active_input_buffer, workspaces[active_tws_id].input_buffer);

  // Clear and redraw
  pufu_terminal_clear_screen();
  printf("%s", workspaces[active_tws_id].history);
  printf("%s%s", active_prompt, active_input_buffer);
  fflush(stdout);
}

// Boot Log Buffer (4KB)
static char boot_log_buffer[4096] = "";
static int boot_log_pos = 0;

const char *pufu_get_boot_logs(void) { return boot_log_buffer; }

void pufu_log(const char *fmt, ...) {
  // 1. Clear current line
  printf("\r\033[K");

  // Format the message
  char msg[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  // 0. Append to Ring Buffer (Crash Log)
  pufu_logger_append(msg);

  // 1. Buffer the message (Retroactive Log & TWS History)
  // Boot Log (Legacy)
  int len = strlen(msg);
  if (boot_log_pos + len + 2 < 4096) {
    strcat(boot_log_buffer, msg);
    strcat(boot_log_buffer, "\n");
    boot_log_pos += len + 1;
  }

  // TWS History (Active Workspace)
  PufuTerminalSpace *tws = &workspaces[active_tws_id];
  if (tws->history_pos + len + 2 < TWS_HISTORY_SIZE) {
    strcat(tws->history, msg);
    strcat(tws->history, "\r\n"); // Use CRLF for history
    tws->history_pos += len + 2;
  } else {
    // Circular buffer or truncate?
    // For simplified implementation, we reset if full (or just stop logging)
    // To keep it simple: just stop logging to history if full
  }

  // 3. Print to stdout
  printf("%s\r\n", msg);

  // 4. Restore prompt and buffer
  printf("%s%s", active_prompt, active_input_buffer);
  fflush(stdout);
}

void pufu_terminal_set_prompt(const char *prompt) {
  strncpy(active_prompt, prompt, 63);
  active_prompt[63] = '\0';
  // Sync with active workspace
  strcpy(workspaces[active_tws_id].prompt, active_prompt);
}

void pufu_terminal_update_buffer(const char *buffer) {
  strncpy(active_input_buffer, buffer, 255);
  active_input_buffer[255] = '\0';
  // No need to sync every keystroke, we sync on switch
}

void pufu_terminal_clear_screen(void) {
  printf("\033[H\033[J"); // ANSI Clear Screen
  fflush(stdout);
}

int pufu_tws_get_active(void) { return active_tws_id; }

void pufu_tws_log(int tws_id, const char *fmt, ...) {
  // Format the message
  char msg[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  // Buffer the message to the specific TWS History
  if (tws_id >= 0 && tws_id < MAX_TWS) {
    PufuTerminalSpace *tws = &workspaces[tws_id];
    int len = strlen(msg);
    if (tws->history_pos + len + 2 < TWS_HISTORY_SIZE) {
      strcat(tws->history, msg);
      strcat(tws->history, "\r\n");
      tws->history_pos += len + 2;
    }
  }

  // Only print to stdout if this TWS is active
  if (tws_id == active_tws_id) {
    // SCROLLING LOGIC:
    // 1. Move cursor to "Current Row" (We assume infinite scroll for now,
    //    but realistically we should use ANSI scroll region).
    //    Simplified: Clear Line -> Print Log -> Newline -> Reprint Prompt.

    // Clear current line (where prompt might be)
    printf("\r\033[K");

    // Print the log line
    printf("%s\r\n", msg);

    // Re-print the Footer (Prompt + Input Buffer)
    // "Prompt> Input"
    printf("%s%s", active_prompt, active_input_buffer);

    fflush(stdout);
  }
}

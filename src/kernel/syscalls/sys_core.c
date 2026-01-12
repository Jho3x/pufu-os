#include "sys_core.h"
#include "pufu/dyn_loader.h"
#include "pufu/terminal.h"
#include "pufu/trinity.h" // For logs?
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Shared helper
void clean_string_arg(char *dest, const char *src) {
  if (src[0] == '"') {
    strncpy(dest, src + 1, 255);
    dest[255] = '\0';
    size_t len = strlen(dest);
    if (len > 0 && dest[len - 1] == '"')
      dest[len - 1] = '\0';
  } else {
    strncpy(dest, src, 255);
    dest[255] = '\0';
  }
}

// Forward Declaration for external deps if needed
// Assuming node->tws_id, pufu_tws_log, etc are available via headers.
// We need headers for process management? pufu/engine.h usually covers
// PufuNode.

int sys_write(PufuNode *node, PufuInstruction *inst) {
  char temp_msg[256];
  clean_string_arg(temp_msg, inst->reg1);
  pufu_tws_log(node->tws_id, "%s", temp_msg);
  node->ip++;
  return 1;
}

int sys_read_char(PufuNode *node, PufuInstruction *inst) {
  // Requires get_reg_index (Assuming it's available or we rewrite it)
  // dispatch.c didn't export it. We might need to copy/move it or include
  // internal. For now, let's implement get_reg_index logic locally or making it
  // shared. Actually, get_reg_index is static in dispatch.c. We should move it
  // to a shared utils. Let's assume we copy functionality for now.

  // Simplified logic: strict parsing "r0"..."r8"
  int reg = -1;
  if (inst->reg1[0] == 'r' && inst->reg1[1] >= '0' && inst->reg1[1] <= '9') {
    reg = inst->reg1[1] - '0';
  }

  if (reg >= 0) {
    int ch = pufu_terminal_get_key();
    node->registers[reg] = (ch > 0) ? ch : 0;
  }
  node->ip++;
  return 1;
}

// ... sys_console_input is huge. I'll condense or move verbatim.
// get_reg_index is provided by node.h/node_exec.c

int sys_console_input(PufuNode *node, PufuInstruction *inst) {
  int reg = (inst->reg1[0]) ? get_reg_index(inst->reg1) : 0;
  int ch = pufu_terminal_get_key();
  if (ch > 0) {
    if (ch == 10 || ch == 13) {
      node->input_buffer[node->input_pos] = 0;
      printf("\r\n");
      pufu_terminal_update_buffer("");
      if (reg >= 0)
        node->registers[reg] = 1;
      node->ip++;
      return 1;
    } else if (ch == 127) {
      if (node->input_pos > 0) {
        node->input_pos--;
        printf("\b \b");
        fflush(stdout);
        node->input_buffer[node->input_pos] = 0;
        pufu_terminal_update_buffer(node->input_buffer);
      }
      if (reg >= 0)
        node->registers[reg] = 0;
    } else {
      if (node->input_pos < 255) {
        node->input_buffer[node->input_pos++] = ch;
        node->input_buffer[node->input_pos] = 0;
        putchar(ch);
        fflush(stdout);
        pufu_terminal_update_buffer(node->input_buffer);
      }
      if (reg >= 0)
        node->registers[reg] = 0;
    }
  } else {
    if (reg >= 0)
      node->registers[reg] = 0;
  }
  node->ip++;
  return 1;
}

// Helper
// get_reg_index is provided by node.h/node_exec.c

int sys_print_char(PufuNode *node, PufuInstruction *inst) {
  int val = get_value(node, inst->reg1);
  putchar(val);
  fflush(stdout);
  node->ip++;
  return 1;
}

int sys_clear_buffer(PufuNode *node) {
  node->input_pos = 0;
  node->input_buffer[0] = 0;
  node->ip++;
  return 1;
}

int sys_set_buffer(PufuNode *node, PufuInstruction *inst) {
  char val[256];
  clean_string_arg(val, inst->reg1);
  // Debug
  // printf("[KERNEL] SYS_SET_BUFFER: '%s'\n", val);
  strncpy(node->input_buffer, val, 255);
  node->input_buffer[255] = 0;
  node->ip++;
  return 1;
}

int sys_log_buffer(PufuNode *node) {
  pufu_tws_log(node->tws_id, "%s", node->input_buffer);
  node->ip++;
  return 1;
}

int sys_string_cmp(PufuNode *node, PufuInstruction *inst) {
  char target[256];
  clean_string_arg(target, inst->reg1);

  // Trim newlines
  char *newline = strchr(node->input_buffer, '\n');
  if (newline)
    *newline = '\0';
  newline = strchr(node->input_buffer, '\r');
  if (newline)
    *newline = '\0';

  if (strcmp(node->input_buffer, target) == 0) {
    node->registers[0] = 1;
  } else {
    node->registers[0] = 0;
  }
  node->ip++;
  return 1;
}

int sys_console_clear(PufuNode *node) {
  pufu_terminal_clear_screen();
  node->ip++;
  return 1;
}

int sys_set_prompt(PufuNode *node, PufuInstruction *inst) {
  char p[256];
  clean_string_arg(p, inst->reg1);
  pufu_terminal_set_prompt(p);
  node->ip++;
  return 1;
}

int sys_cat(PufuNode *node, PufuInstruction *inst) {
  char filename[256];
  clean_string_arg(filename, inst->reg1);
  FILE *f = fopen(filename, "r");
  if (f) {
    char line[256];
    while (fgets(line, sizeof(line), f)) {
      line[strcspn(line, "\n")] = 0;
      pufu_log("%s", line);
    }
    fclose(f);
  } else {
    pufu_log("Error: Could not open file %s", filename);
  }
  node->ip++;
  return 1;
}

int sys_config_get(PufuNode *node, PufuInstruction *inst) {
  char key[256]; // Was 64, caused stack smash via clean_string_arg (255 bytes)
  clean_string_arg(key, inst->reg1);
  printf("[DEBUG] sys_config_get: Key='%s'\n", key);
  node->input_buffer[0] = 0;
  FILE *f = fopen("user_config.pufu", "r");
  if (f) {
    char line[256];
    while (fgets(line, sizeof(line), f)) {
      char *colon = strchr(line, ':');
      if (colon) {
        *colon = 0;
        char *k = line;
        char *v = colon + 1;
        while (*k == ' ')
          k++;
        // Trim trailing spaces from k?
        // Basic impl:
        if (strcmp(k, key) == 0) {
          while (*v == ' ')
            v++;
          v[strcspn(v, "\n")] = 0;
          v[strcspn(v, "\r")] = 0; // Handle CRLF
          strncpy(node->input_buffer, v, 255);
          printf("[DEBUG] Found value: '%s'\n", node->input_buffer);
          break;
        }
      }
    }
    fclose(f);
  } else {
    printf("[DEBUG] Failed to open user_config.pufu\n");
  }
  node->ip++;
  return 1;
}

int sys_prepend_string(PufuNode *node, PufuInstruction *inst) {
  char prefix[256];
  clean_string_arg(prefix, inst->reg1);
  char temp[512];
  snprintf(temp, sizeof(temp), "%s%s", prefix, node->input_buffer);
  strncpy(node->input_buffer, temp, 255);
  node->ip++;
  return 1;
}

int sys_append_string(PufuNode *node, PufuInstruction *inst) {
  char suffix[256];
  clean_string_arg(suffix, inst->reg1);
  char temp[512];
  snprintf(temp, sizeof(temp), "%s%s", node->input_buffer, suffix);
  strncpy(node->input_buffer, temp, 255);
  node->ip++;
  return 1;
}

int sys_itoa(PufuNode *node) {
  int val = node->registers[1];
  snprintf(node->input_buffer, 255, "%d", val);
  node->ip++;
  return 1;
}

int sys_get_version(PufuNode *node) {
  PufuSocket *s = pufu_get_current_socket(); // Ensure we have header for this
                                             // or forward decl
  if (s && s->syscall) {
    s->syscall(SYS_GET_VERSION, NULL);
  }
  strncpy(node->input_buffer, "0.5.2 (Peking Duck)", 255);
  node->ip++;
  return 1;
}

int sys_system_update(PufuNode *node, PufuInstruction *inst) {
  char path[256];
  clean_string_arg(path, inst->reg1);
  printf("[KERNEL] Requesting System Update -> %s\n", path);

  if (pufu_reload_socket(path) < 0) {
    node->registers[0] = 0; // Fail
  } else {
    node->registers[0] = 1; // Success
  }
  node->ip++;
  return 1;
}

int sys_download_update(PufuNode *node, PufuInstruction *inst) {
  char url[256];
  clean_string_arg(url, inst->reg1);
  char dest[256] = "/tmp/pufu_update.so"; // Default temp path

  // Optional: Allow reg2 to specify dest, but for now fixed.

  if (pufu_dyn_loader_download(url, dest) < 0) {
    node->registers[0] = 0;
  } else {
    node->registers[0] = 1;
  }
  node->ip++;
  return 1;
}

int sys_core_dispatch(PufuNode *node, PufuInstruction *inst) {
  switch (inst->syscall_id) {
  case SYS_WRITE:
    return sys_write(node, inst);
  case SYS_READ_CHAR:
    return sys_read_char(node, inst);
  case SYS_CONSOLE_INPUT:
    return sys_console_input(node, inst);
  case SYS_PRINT_CHAR:
    return sys_print_char(node, inst);
  case SYS_CLEAR_BUFFER:
    return sys_clear_buffer(node);
  case SYS_SET_BUFFER:
    return sys_set_buffer(node, inst);
  case SYS_LOG_BUFFER:
    return sys_log_buffer(node);
  case SYS_STRING_CMP:
    return sys_string_cmp(node, inst);
  case SYS_CONSOLE_CLEAR:
    return sys_console_clear(node);
  case SYS_SET_PROMPT:
    return sys_set_prompt(node, inst);
  case SYS_CAT:
    return sys_cat(node, inst);
  case SYS_CONFIG_GET:
    return sys_config_get(node, inst);
  case SYS_PREPEND_STRING:
    return sys_prepend_string(node, inst);
  case SYS_APPEND_STRING:
    return sys_append_string(node, inst);
  case SYS_ITOA:
    return sys_itoa(node);
  case SYS_GET_VERSION:
    return sys_get_version(node);
  case SYS_SYSTEM_UPDATE:
    return sys_system_update(node, inst);
  case SYS_DOWNLOAD_UPDATE:
    return sys_download_update(node, inst);
  default:
    return 0;
  }
}

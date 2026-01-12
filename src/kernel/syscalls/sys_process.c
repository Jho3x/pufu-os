#include "sys_process.h"
#include "pufu/terminal.h"
#include "pufu/trinity.h" // For logs?
#include "sys_core.h"     // For clean_string_arg
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Helper functions provided by node.h/node_exec.c or terminal.h

int sys_spawn(PufuNodeSystem *sys, PufuNode *node, PufuInstruction *inst) {
  char filename[256];
  clean_string_arg(filename, inst->reg1);
  pufu_tws_log(node->tws_id, "Syscall (spawn): Starting %s...", filename);
  PufuNode *child = pufu_node_load(sys, filename);
  if (child)
    child->tws_id = node->tws_id;
  node->ip++;
  return 1;
}

int sys_exec(PufuNodeSystem *sys, PufuNode *node, PufuInstruction *inst) {
  char filename[256];
  clean_string_arg(filename, inst->reg1);
  pufu_tws_log(node->tws_id, "Syscall (exec): Chain loading %s...", filename);
  PufuNode *child = pufu_node_load(sys, filename);
  if (child)
    child->tws_id = node->tws_id;

  // Reset Prompt/Buffer if switching apps
  // Assuming pufu_terminal_set_prompt exists globally or included
  // pufu_terminal_set_prompt("");
  // pufu_terminal_update_buffer("");

  node->active = 0;
  node->ip++;
  return 1;
}

int sys_kill(PufuNodeSystem *sys, PufuNode *node, PufuInstruction *inst) {
  char filename[256];
  clean_string_arg(filename, inst->reg1);
  pufu_tws_log(node->tws_id, "Syscall (kill): Stopping %s...", filename);
  PufuNode *target = sys->nodes;
  while (target) {
    if (strcmp(target->filename, filename) == 0) {
      target->active = 0;
    }
    target = target->next;
  }
  node->ip++;
  return 1;
}

int sys_exit(PufuNode *node) {
  pufu_tws_log(node->tws_id, "Syscall (exit): Terminating.");
  node->active = 0;
  node->ip++;
  return 1;
}

int sys_shutdown(PufuNode *node) {
  pufu_tws_log(node->tws_id, "Syscall: Shutdown requested.");
  pufu_os_shutdown();
  return 1;
}

int sys_sleep(PufuNode *node, PufuInstruction *inst) {
  int ms = atoi(inst->reg1);
  node->wake_time = get_time_ms() + ms;
  node->ip++;
  return 1;
}

int sys_spawn_from_buffer(PufuNodeSystem *sys, PufuNode *node) {
  pufu_tws_log(node->tws_id, "Syscall (spawn): Starting %s...",
               node->input_buffer);
  PufuNode *child = pufu_node_load(sys, node->input_buffer);
  if (child)
    child->tws_id = node->tws_id;
  node->ip++;
  return 1;
}

int sys_kill_from_buffer(PufuNodeSystem *sys, PufuNode *node) {
  pufu_log("Syscall (kill): Stopping %s...", node->input_buffer);
  PufuNode *target = sys->nodes;
  while (target) {
    if (strcmp(target->filename, node->input_buffer) == 0) {
      target->active = 0;
    }
    target = target->next;
  }
  node->ip++;
  return 1;
}

// Helper needed? get_reg_index should be available.
// I'll inline a static one here or move to common utils later.
static int get_reg_index_local(const char *r) {
  if (r[0] == 'r' && r[1] >= '0' && r[1] <= '7') {
    return r[1] - '0';
  }
  return -1;
}

int sys_parse_command(PufuNode *node, PufuInstruction *inst) {
  int reg = get_reg_index_local(inst->reg1);
  if (reg >= 0) {
    if (strncmp(node->input_buffer, "init ", 5) == 0) {
      memmove(node->input_buffer, node->input_buffer + 5, 256 - 5);
      node->registers[reg] = 1;
    } else if (strncmp(node->input_buffer, "stop ", 5) == 0) {
      memmove(node->input_buffer, node->input_buffer + 5, 256 - 5);
      node->registers[reg] = 2;
    } else if (strncmp(node->input_buffer, "command ", 8) == 0) {
      memmove(node->input_buffer, node->input_buffer + 8, 256 - 8);
      node->registers[reg] = 3;
    } else if (strncmp(node->input_buffer, "shutdown", 8) == 0) {
      node->registers[reg] = 4;
    } else if (strncmp(node->input_buffer, "tws ", 4) == 0) {
      // Warning: Duplicate logic in original dispatch.c?
      // Treating as 4? original said 4. Then 6?
      // Keeping as is: 4.
      node->registers[reg] = 4;
    } else {
      node->registers[reg] = 0;
    }
  }
  node->ip++;
  return 1;
}

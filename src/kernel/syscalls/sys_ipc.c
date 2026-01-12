#include "sys_ipc.h"
#include "pufu/engine.h"
#include <string.h>

// Assuming PUFU_IPC_QUEUE_SIZE is defined in engine.h or similar?
// Usually defined in `virtual_bus.h` or `engine.h`
// Let's assume it's 32 or check header.

// Helper for get_reg_index (local)
// get_reg_index is provided by node.h/node_exec.c

int sys_ipc_send(PufuNodeSystem *sys, PufuNode *node, PufuInstruction *inst) {
  (void)inst;
  char *colon = strchr(node->input_buffer, ':');
  if (colon) {
    *colon = 0;
    char *target_name = node->input_buffer;
    char *message = colon + 1;
    while (*message == ' ')
      message++;

    PufuNode *t = sys->nodes;
    while (t) {
      if (strcmp(t->filename, target_name) == 0) {
        if (t->ipc_count < 32) { // Use default size
          int h = t->ipc_head;
          strncpy(t->ipc_queue[h].sender, node->filename, 63);
          strncpy(t->ipc_queue[h].content, message, 191);
          t->ipc_queue[h].type = 0;
          t->ipc_head = (h + 1) % 32;
          t->ipc_count++;
        }
        break;
      }
      t = t->next;
    }
  }
  node->ip++;
  return 1;
}

int sys_ipc_read(PufuNode *node, PufuInstruction *inst) {
  int reg = get_reg_index(inst->reg1);
  if (node->ipc_count > 0) {
    int t = node->ipc_tail;
    strncpy(node->input_buffer, node->ipc_queue[t].content, 255);
    node->ipc_tail = (t + 1) % 32;
    node->ipc_count--;
    if (reg >= 0)
      node->registers[reg] = 1;
  } else {
    if (reg >= 0)
      node->registers[reg] = 0;
  }
  node->ip++;
  return 1;
}

int sys_ipc_broadcast(PufuNodeSystem *sys, PufuNode *node) {
  char *msg = node->input_buffer;
  PufuNode *t = sys->nodes;
  while (t) {
    if (t != node && t->active) {
      if (t->ipc_count < 32) {
        int h = t->ipc_head;
        strncpy(t->ipc_queue[h].sender, node->filename, 63);
        strncpy(t->ipc_queue[h].content, msg, 191);
        t->ipc_queue[h].type = 2; // Broadcast Type
        t->ipc_head = (h + 1) % 32;
        t->ipc_count++;
      }
    }
    t = t->next;
  }
  node->ip++;
  return 1;
}

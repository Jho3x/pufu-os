#include "pufu/crystal.h"
#include "pufu/hot_reload.h" // For system_check
#include "pufu/node.h"
#include "pufu/socket.h"
#include "pufu/syscalls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helpers para VM
int get_reg_index(const char *str) {
  if (str[0] == 'r' || str[0] == 'R') {
    return atoi(str + 1);
  }
  return -1;
}

int get_value(PufuNode *node, const char *str) {
  int reg = get_reg_index(str);
  if (reg >= 0 && reg < 16) {
    return node->registers[reg];
  }
  return atoi(str);
}

int find_label(PufuNode *node, const char *label) {
  if (!node->parser)
    return -1;

  // OPTIMIZED LOOKUP: Use Label Index
  if (node->parser->labels) {
    for (int i = 0; i < node->parser->label_count; i++) {
      if (strcmp(node->parser->labels[i].name, label) == 0) {
        return node->parser->labels[i].index;
      }
    }
  }
  return -1;
}

// Ejecutar un nodo (un paso)
int pufu_node_execute(PufuNodeSystem *sys, PufuNode *node) {
  if (!node || !node->active)
    return 0;

  // Verificar si está durmiendo
  if (node->wake_time > 0) {
    if (get_time_ms() < node->wake_time) {
      return 1; // Sigue durmiendo
    }
    node->wake_time = 0; // Despertar
  }

  if (node->type == PUFU_NODE_ASSEMBLER ||
      node->type == PUFU_NODE_TRINITY_SCENE) {
    if (!node->parser || node->ip >= node->parser->count) {
      node->active = 0;
      return 0;
    }

    PufuInstruction *inst = &node->parser->instructions[node->ip];
    char code[256];

    // OPTIMIZED VM: Switch by Opcode
    switch (inst->opcode) {
    case OP_MOV: {
      int reg = get_reg_index(inst->reg1);
      if (reg >= 0) {
        const char *src = inst->reg2[0] ? inst->reg2 : inst->value;
        node->registers[reg] = get_value(node, src);
      }
      node->ip++;
      return 1;
    }

    case OP_ADD: {
      int reg = get_reg_index(inst->reg1);
      if (reg >= 0) {
        const char *src = inst->reg2[0] ? inst->reg2 : inst->value;
        PufuSocket *socket = pufu_get_current_socket();
        if (socket && socket->alu_add) {
          node->registers[reg] =
              socket->alu_add(node->registers[reg], get_value(node, src));
        }
      }
      node->ip++;
      return 1;
    }

    case OP_SUB: {
      int reg = get_reg_index(inst->reg1);
      if (reg >= 0) {
        const char *src = inst->reg2[0] ? inst->reg2 : inst->value;
        PufuSocket *socket = pufu_get_current_socket();
        if (socket && socket->alu_sub) {
          node->registers[reg] =
              socket->alu_sub(node->registers[reg], get_value(node, src));
        }
      }
      node->ip++;
      return 1;
    }

    case OP_CMP: {
      int v1 = get_value(node, inst->reg1);
      const char *src = inst->reg2[0] ? inst->reg2 : inst->value;
      int v2 = get_value(node, src);
      PufuSocket *socket = pufu_get_current_socket();
      if (socket && socket->alu_cmp) {
        node->cmp_flag = socket->alu_cmp(v1, v2);
      }
      node->ip++;
      return 1;
    }

    case OP_JMP: {
      int target = find_label(node, inst->reg1);
      if (target >= 0)
        node->ip = target;
      else
        node->ip++;
      return 1;
    }

    case OP_BEQ: {
      if (node->cmp_flag == 0) {
        int target = find_label(node, inst->reg1);
        if (target >= 0) {
          node->ip = target;
          return 1;
        }
      }
      node->ip++;
      return 1;
    }

    case OP_BNE: {
      if (node->cmp_flag != 0) {
        int target = find_label(node, inst->reg1);
        if (target >= 0) {
          node->ip = target;
          return 1;
        }
      }
      node->ip++;
      return 1;
    }

    case OP_BLT: {
      if (node->cmp_flag == -1) {
        int target = find_label(node, inst->reg1);
        if (target >= 0) {
          node->ip = target;
          return 1;
        }
      }
      node->ip++;
      return 1;
    }

    case OP_BGT: {
      if (node->cmp_flag == 1) {
        int target = find_label(node, inst->reg1);
        if (target >= 0) {
          node->ip = target;
          return 1;
        }
      }
      node->ip++;
      return 1;
    }

    case OP_LABEL:
      node->ip++;
      return 1;

    case OP_SYSCALL:
      // Check Dispatch
      if (pufu_syscall_dispatch(sys, node, inst)) {
        return 1;
      }
      // Fallback to ARM Socket text exec for unknown syscalls?
      // "syscall name arg1"
      snprintf(code, sizeof(code), "syscall %s", inst->value);
      if (inst->reg1[0]) {
        strncat(code, " ", 255 - strlen(code));
        strncat(code, inst->reg1, 255 - strlen(code));
      }
      PufuSocket *socket = pufu_get_current_socket();
      if (socket && socket->execute) {
        socket->execute(code);
      }
      node->ip++;
      return 1;

    default:
      // NOP or Unknown
      node->ip++; // Just skip
      return 1;
    }
  } else if (node->type == PUFU_NODE_CRYSTAL) {
    if (node->crystal) {
      uint8_t result = pufu_crystal_step(node->crystal);
      (void)result;
      // printf("Crystal Output: %d\n", result);
    }
    return 1;
  }

  return 0;
}

// Verificar cambios en todos los nodos
int pufu_node_system_check(PufuNodeSystem *system) {
  if (!system)
    return -1;

  int changes = 0;
  PufuNode *current = system->nodes;
  while (current) {
    int result = 0;
    if (current->reload) {
      result = pufu_hot_reload_check(current->reload);
    }
    if (result > 0) {
      printf("\nNodo modificado: %s\n", current->filename);
      if (current->is_arbiter) {
        printf("(Este es el nodo árbitro)\n");
      }
      changes++;
    }
    current = current->next;
  }

  return changes;
}

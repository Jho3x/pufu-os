#include "pufu/parser.h"
#include "pufu/syscall_ids.h" // New Header
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 32

PufuParser *pufu_parser_init(void) {
  PufuParser *parser = malloc(sizeof(PufuParser));
  if (!parser)
    return NULL;

  parser->instructions = malloc(sizeof(PufuInstruction) * INITIAL_CAPACITY);
  if (!parser->instructions) {
    free(parser);
    return NULL;
  }

  parser->count = 0;
  parser->capacity = INITIAL_CAPACITY;
  parser->has_pufu_init = 0;

  // Label Index Init
  parser->label_capacity = 16;
  parser->labels = malloc(sizeof(PufuLabelEntry) * parser->label_capacity);
  parser->label_count = 0;

  return parser;
}

static int is_comment_or_empty(const char *line) {
  while (*line) {
    if (*line == '#')
      return 1;
    if (!isspace(*line))
      return 0;
    line++;
  }
  return 1;
}

static int is_label(const char *line) {
  if (strstr(line, "_claw::") || strstr(line, "_pufu::"))
    return 0;
  const char *ptr = line;
  while (*ptr && !isspace(*ptr))
    ptr++;
  if (ptr > line && *(ptr - 1) == ':')
    return 1;
  return 0;
}

static void get_opcode_from_string(PufuInstruction *inst, const char *op_str) {
  if (strcmp(op_str, "mov") == 0)
    inst->opcode = OP_MOV;
  else if (strcmp(op_str, "add") == 0)
    inst->opcode = OP_ADD;
  else if (strcmp(op_str, "sub") == 0)
    inst->opcode = OP_SUB;
  else if (strcmp(op_str, "mul") == 0)
    inst->opcode = OP_MUL;
  else if (strcmp(op_str, "div") == 0)
    inst->opcode = OP_DIV;
  else if (strcmp(op_str, "cmp") == 0)
    inst->opcode = OP_CMP;
  else if (strcmp(op_str, "jmp") == 0)
    inst->opcode = OP_JMP;
  else if (strcmp(op_str, "beq") == 0)
    inst->opcode = OP_BEQ;
  else if (strcmp(op_str, "bne") == 0)
    inst->opcode = OP_BNE;
  else if (strcmp(op_str, "blt") == 0)
    inst->opcode = OP_BLT;
  else if (strcmp(op_str, "bgt") == 0)
    inst->opcode = OP_BGT;
  else if (strcmp(op_str, "label") == 0)
    inst->opcode = OP_LABEL;
  else if (strcmp(op_str, "syscall") == 0)
    inst->opcode = OP_SYSCALL;
  else if (op_str[0] == '(')
    inst->opcode = OP_SYSCALL; // Shorthand
  else
    inst->opcode = OP_NOP;
}

// Optimized Syscall Tokenization
static int get_syscall_id(const char *name) {
  if (strcmp(name, "(write)") == 0)
    return SYS_WRITE;
  if (strcmp(name, "(read_char)") == 0)
    return SYS_READ_CHAR;
  if (strcmp(name, "(console_input)") == 0)
    return SYS_CONSOLE_INPUT;
  if (strcmp(name, "(print_char)") == 0)
    return SYS_PRINT_CHAR;
  if (strcmp(name, "(clear_buffer)") == 0)
    return SYS_CLEAR_BUFFER;
  if (strcmp(name, "(console_clear)") == 0)
    return SYS_CONSOLE_CLEAR;
  if (strcmp(name, "(set_prompt)") == 0)
    return SYS_SET_PROMPT;
  if (strcmp(name, "(cat)") == 0)
    return SYS_CAT;
  if (strcmp(name, "(log_buffer)") == 0)
    return SYS_LOG_BUFFER;
  if (strcmp(name, "(prepend_string)") == 0)
    return SYS_PREPEND_STRING;

  if (strcmp(name, "(spawn)") == 0)
    return SYS_SPAWN;
  if (strcmp(name, "(exec)") == 0)
    return SYS_EXEC;
  if (strcmp(name, "(kill)") == 0)
    return SYS_KILL;
  if (strcmp(name, "(exit)") == 0)
    return SYS_EXIT;
  if (strcmp(name, "(sleep)") == 0)
    return SYS_SLEEP;
  if (strcmp(name, "(shutdown)") == 0)
    return SYS_SHUTDOWN;
  if (strcmp(name, "(spawn_from_buffer)") == 0)
    return SYS_SPAWN_FROM_BUFFER;
  if (strcmp(name, "(kill_from_buffer)") == 0)
    return SYS_KILL_FROM_BUFFER;
  if (strcmp(name, "(parse_command)") == 0)
    return SYS_PARSE_COMMAND;

  if (strcmp(name, "(ipc_send_from_buffer)") == 0)
    return SYS_IPC_SEND;
  if (strcmp(name, "(ipc_read)") == 0)
    return SYS_IPC_READ;
  if (strcmp(name, "(ipc_broadcast_from_buffer)") == 0)
    return SYS_IPC_BROADCAST;

  if (strcmp(name, "(config_get)") == 0)
    return SYS_CONFIG_GET;

  if (strcmp(name, "(tws_switch)") == 0)
    return SYS_TWS_SWITCH;
  if (strcmp(name, "(tws_switch_from_args)") == 0)
    return SYS_TWS_SWITCH_ARGS;

  if (strcmp(name, "(trinity_init)") == 0)
    return SYS_TRINITY_INIT;
  if (strcmp(name, "(trinity_step)") == 0)
    return SYS_TRINITY_STEP;
  if (strcmp(name, "(window_init)") == 0)
    return SYS_WINDOW_INIT;
  if (strcmp(name, "(window_clear)") == 0)
    return SYS_WINDOW_CLEAR;
  if (strcmp(name, "(window_swap)") == 0)
    return SYS_WINDOW_SWAP;
  if (strcmp(name, "(window_draw_model)") == 0)
    return SYS_WINDOW_DRAW_MODEL;

  if (strcmp(name, "(create_ui_button)") == 0)
    return SYS_CREATE_UI_BUTTON;

  if (strcmp(name, "(trinity_set_vec3)") == 0)
    return SYS_TRINITY_SET_VEC3;

  if (strcmp(name, "(trinity_set_string)") == 0)
    return SYS_TRINITY_SET_STRING;

  if (strcmp(name, "(create_ui_image)") == 0)
    return SYS_CREATE_UI_IMAGE;

  if (strcmp(name, "(create_ui_window)") == 0)
    return SYS_CREATE_UI_WINDOW;

  if (strcmp(name, "(trinity_poll_event)") == 0)
    return SYS_TRINITY_POLL_EVENT;
  if (strcmp(name, "(trinity_set_vec4)") == 0)
    return SYS_TRINITY_SET_VEC4;
  if (strcmp(name, "(trinity_get_vec4)") == 0)
    return SYS_TRINITY_GET_VEC4;
  if (strcmp(name, "(trinity_update_rect)") == 0)
    return SYS_TRINITY_UPDATE_RECT;

  // Meow
  if (strcmp(name, "(trinity_load_meow)") == 0)
    return SYS_TRINITY_LOAD_MEOW;
  if (strcmp(name, "load_meow") == 0)
    return SYS_TRINITY_LOAD_MEOW;
  if (strcmp(name, "(itoa)") == 0)
    return SYS_ITOA;
  if (strcmp(name, "(bind_event)") == 0)
    return SYS_BIND_EVENT;
  if (strcmp(name, "(exec_binding)") == 0)
    return SYS_EXEC_BINDING;
  if (strcmp(name, "(get_version)") == 0)
    return SYS_GET_VERSION;
  if (strcmp(name, "(clear_buffer)") == 0)
    return SYS_CLEAR_BUFFER;

  if (strcmp(name, "(system_update)") == 0)
    return SYS_SYSTEM_UPDATE;
  if (strcmp(name, "(download_update)") == 0)
    return SYS_DOWNLOAD_UPDATE;

  return SYS_UNKNOWN;
}

int pufu_parse_line(PufuParser *parser, const char *line) {
  if (is_comment_or_empty(line))
    return 0;

  if (parser->count >= parser->capacity) {
    int new_capacity = parser->capacity * 2;
    PufuInstruction *new_instructions =
        realloc(parser->instructions, sizeof(PufuInstruction) * new_capacity);
    if (!new_instructions)
      return -1;
    parser->instructions = new_instructions;
    parser->capacity = new_capacity;
  }

  PufuInstruction *inst = &parser->instructions[parser->count];
  memset(inst, 0, sizeof(PufuInstruction));

  // Handle Label Definition Syntax "name:"
  if (is_label(line)) {
    inst->opcode = OP_LABEL;
    strcpy(inst->op, "label");
    const char *ptr = line;
    while (*ptr && isspace(*ptr))
      ptr++;
    const char *end = ptr;
    while (*end && *end != ':')
      end++;
    int len = end - ptr;
    if (len >= 127)
      len = 127;
    strncpy(inst->reg1, ptr, len);
    inst->reg1[len] = '\0';

    // START LABEL INDEXING LOGIC
    if (parser->label_count >= parser->label_capacity) {
      int new_cap = parser->label_capacity * 2;
      PufuLabelEntry *new_labels =
          realloc(parser->labels, sizeof(PufuLabelEntry) * new_cap);
      if (new_labels) {
        parser->labels = new_labels;
        parser->label_capacity = new_cap;
      }
    }
    if (parser->labels && parser->label_count < parser->label_capacity) {
      strncpy(parser->labels[parser->label_count].name, inst->reg1, 63);
      parser->labels[parser->label_count].index = parser->count;
      parser->label_count++;
    }
    // END LABEL INDEXING LOGIC

    parser->count++;
    return 0;
  }

  // Normal Instruction Parsing
  char buffer[256];
  strncpy(buffer, line, 255);
  buffer[255] = '\0';

  char *ptr = buffer;
  while (*ptr && isspace(*ptr))
    ptr++;

  // Op
  char *op_start = ptr;
  while (*ptr && !isspace(*ptr))
    ptr++;
  if (*ptr) {
    *ptr = 0;
    ptr++;
  }

  strncpy(inst->op, op_start, 31);
  get_opcode_from_string(inst, inst->op);

  // Special Handling for Syscall Shorthand "(write)"
  if (inst->opcode == OP_SYSCALL && inst->op[0] == '(') {
    strncpy(inst->value, inst->op, 127);
    inst->is_syscall = 1;
    inst->syscall_id = get_syscall_id(inst->value);
    // Continue to parse args as reg1
  }

  // Arg 1
  while (*ptr && isspace(*ptr))
    ptr++;
  if (*ptr) {
    char *arg1_start = ptr;
    if (*ptr == '"') {
      ptr++; // skip quote
      while (*ptr && *ptr != '"')
        ptr++;
      if (*ptr)
        ptr++; // skip closing
    } else {
      while (*ptr && !isspace(*ptr))
        ptr++;
    }
    char save = *ptr;
    *ptr = 0;
    strncpy(inst->reg1, arg1_start, 127);

    // If it's a syscall, and we parsed arg1, this might be the value (if
    // explicit syscall)
    if (strcmp(inst->op, "syscall") == 0) {
      strncpy(inst->value, arg1_start, 127); // value = syscall name
      inst->is_syscall = 1;
      inst->syscall_id = get_syscall_id(inst->value);
      *inst->reg1 = 0;
    }

    if (save)
      ptr++;

    // PATCH: Index OP_LABEL style labels
    if (inst->opcode == OP_LABEL) {
      if (parser->label_count >= parser->label_capacity) {
        int new_cap = parser->label_capacity * 2;
        PufuLabelEntry *new_labels =
            realloc(parser->labels, sizeof(PufuLabelEntry) * new_cap);
        if (new_labels) {
          parser->labels = new_labels;
          parser->label_capacity = new_cap;
        }
      }
      if (parser->labels && parser->label_count < parser->label_capacity) {
        strncpy(parser->labels[parser->label_count].name, inst->reg1, 63);
        parser->labels[parser->label_count].index = parser->count;
        parser->label_count++;
      }
    }
  }

  // Arg 2 (or Arg 1 for syscall)
  while (*ptr && isspace(*ptr))
    ptr++;
  if (*ptr) {
    char *arg2_start = ptr;
    if (*ptr == '"') {
      ptr++;
      while (*ptr && *ptr != '"')
        ptr++;
      if (*ptr)
        ptr++;
    } else {
      while (*ptr && !isspace(*ptr))
        ptr++;
    }
    char save = *ptr;
    *ptr = 0;

    if (inst->is_syscall) {
      strncpy(inst->reg1, arg2_start, 127);
    } else {
      strncpy(inst->reg2, arg2_start, 127);
      if (isdigit(arg2_start[0]) || arg2_start[0] == '-') {
        strncpy(inst->value, arg2_start, 127);
      }
    }
    if (save)
      ptr++;
  }

  parser->count++;
  return 0;
}

int pufu_parse_file(PufuParser *parser, const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file)
    return -1;
  char line[256];
  while (fgets(line, sizeof(line), file)) {
    if (pufu_parse_line(parser, line) < 0) {
      fclose(file);
      return -1;
    }
  }
  fclose(file);
  return 0;
}

void pufu_parser_cleanup(PufuParser *parser) {
  if (parser) {
    if (parser->instructions)
      free(parser->instructions);
    if (parser->labels)
      free(parser->labels);
    free(parser);
  }
}

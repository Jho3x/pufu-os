#include "sys_trinity.h"
#include "pufu/terminal.h"
#include "pufu/trinity.h"
#include "sys_core.h" // clean_string_arg
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sys_trinity_init(PufuNode *node) {
  trinity_init();
  node->ip++;
  return 1;
}

int sys_trinity_step(PufuNode *node) {
  int status = trinity_step();
  node->registers[0] = status;
  node->ip++;
  return 1;
}

int sys_window_init(PufuNode *node, PufuInstruction *inst) {
  char title[256];
  clean_string_arg(title, inst->reg1);
  trinity_create_node(title, NODE_WINDOW);
  trinity_prepare();
  node->registers[0] = 1;
  node->ip++;
  return 1;
}

int sys_create_ui_button(PufuNode *node, PufuInstruction *inst) {
  char name[256];
  clean_string_arg(name, inst->reg1);
  float x, y, w, h;
  if (sscanf(node->input_buffer, "%f %f %f %f", &x, &y, &w, &h) != 4) {
    x = 10;
    y = 10;
    w = 100;
    h = 40;
  }
  NodeID id = trinity_create_node(name, NODE_UI_BUTTON);
  trinity_set_vec4(id, "rect", x, y, w, h);
  node->registers[0] = id;
  node->ip++;
  return 1;
}

int sys_trinity_set_vec3(PufuNode *node, PufuInstruction *inst) {
  int id = -1;
  char first_token[64];
  char prop_name[64];
  char arg_str[256];

  clean_string_arg(arg_str, inst->reg1);
  if (sscanf(arg_str, "%s %s", first_token, prop_name) != 2)
    return 1;

  if (first_token[0] >= '0' && first_token[0] <= '9') {
    id = atoi(first_token);
  } else {
    id = trinity_get_node_id(first_token);
    if (id < 0)
      return 1;
  }

  float x, y, z;
  if (sscanf(node->input_buffer, "%f %f %f", &x, &y, &z) != 3) {
    x = 0;
    y = 0;
    z = 0;
  }
  trinity_set_vec3(id, prop_name, x, y, z);
  printf("[KERNEL] SYS_TRINITY_SET_VEC3: ID=%d Prop='%s' Val=%.2f,%.2f,%.2f\n",
         id, prop_name, x, y, z);
  node->ip++;
  return 1;
}

int sys_trinity_set_string(PufuNode *node, PufuInstruction *inst) {
  int id = -1;
  char first_token[64];
  char prop_name[64];
  char arg_str[256];

  clean_string_arg(arg_str, inst->reg1);
  if (sscanf(arg_str, "%s %s", first_token, prop_name) != 2)
    return 1;

  if (first_token[0] >= '0' && first_token[0] <= '9') {
    id = atoi(first_token);
  } else {
    id = trinity_get_node_id(first_token);
    if (id < 0)
      return 1;
  }

  // FIXED: Removed Double Increment
  trinity_set_string(id, prop_name, node->input_buffer);
  node->ip++;
  return 1;
}

int sys_create_ui_image(PufuNode *node, PufuInstruction *inst) {
  char name[256];
  clean_string_arg(name, inst->reg1);
  float x, y, w, h;
  if (sscanf(node->input_buffer, "%f %f %f %f", &x, &y, &w, &h) != 4) {
    x = 0;
    y = 0;
    w = 32;
    h = 32;
  }
  NodeID id = trinity_create_node(name, NODE_UI_IMAGE);
  trinity_set_vec4(id, "rect", x, y, w, h);
  node->registers[0] = id;
  node->ip++;
  return 1;
}

int sys_create_ui_window(PufuNode *node, PufuInstruction *inst) {
  char name[256];
  clean_string_arg(name, inst->reg1);
  float x, y, w, h;
  if (sscanf(node->input_buffer, "%f %f %f %f", &x, &y, &w, &h) != 4) {
    x = 50;
    y = 50;
    w = 400;
    h = 300;
  }
  NodeID id = trinity_create_node(name, NODE_UI_WINDOW);
  trinity_set_vec4(id, "rect", x, y, w, h);
  node->registers[0] = id;
  node->ip++;
  return 1;
}

int sys_trinity_set_vec4(PufuNode *node, PufuInstruction *inst) {
  int id = -1;
  char first_token[64];
  char prop_name[64];
  char arg_str[256];

  clean_string_arg(arg_str, inst->reg1);
  if (sscanf(arg_str, "%s %s", first_token, prop_name) != 2)
    return 1;

  if (first_token[0] >= '0' && first_token[0] <= '9') {
    id = atoi(first_token);
  } else {
    id = trinity_get_node_id(first_token);
    if (id < 0)
      return 1;
  }

  float x, y, z, w;
  if (sscanf(node->input_buffer, "%f %f %f %f", &x, &y, &z, &w) != 4) {
    x = 0;
    y = 0;
    z = 0;
    w = 0;
  }
  trinity_set_vec4(id, prop_name, x, y, z, w);
  node->ip++;
  return 1;
}

int sys_trinity_get_vec4(PufuNode *node, PufuInstruction *inst) {
  int id = -1;
  char first_token[64];
  char prop_name[64];
  char arg_str[256];

  clean_string_arg(arg_str, inst->reg1);
  if (sscanf(arg_str, "%s %s", first_token, prop_name) != 2)
    return 1;

  if (first_token[0] >= '0' && first_token[0] <= '9') {
    id = atoi(first_token);
  } else {
    id = trinity_get_node_id(first_token);
    if (id < 0) {
      node->registers[1] = 0;
      node->registers[2] = 0;
      return 1;
    }
  }

  float v[4] = {0, 0, 0, 0};
  trinity_get_vec4(id, prop_name, v);
  node->registers[1] = (int)v[0];
  node->registers[2] = (int)v[1];
  node->registers[3] = (int)v[2];
  node->registers[4] = (int)v[3];
  node->ip++;
  return 1;
}

int sys_trinity_update_rect(PufuNode *node, PufuInstruction *inst) {
  int id = -1;
  char arg_str[256];
  clean_string_arg(arg_str, inst->reg1);

  if (arg_str[0] >= '0' && arg_str[0] <= '9') {
    id = atoi(arg_str);
  } else {
    id = trinity_get_node_id(arg_str);
    if (id < 0)
      return 1;
  }

  float x = (float)node->registers[1];
  float y = (float)node->registers[2];
  float z = (float)node->registers[3];
  float w = (float)node->registers[4];

  trinity_set_vec4(id, "rect", x, y, z, w);
  node->ip++;
  return 1;
}

int sys_trinity_poll_event(PufuNode *node) {
  TrinityEvent e;
  if (trinity_dequeue_event(&e)) {
    node->registers[1] = e.type;
    node->registers[2] = e.x;
    node->registers[3] = e.y;
    node->registers[4] = e.target_id;
  } else {
    node->registers[1] = 0;
  }
  node->ip++;
  return 1;
}

int sys_bind_event(PufuNode *node, PufuInstruction *inst) {
  char name[256];
  clean_string_arg(name, inst->reg1);
  NodeID id = -1;
  if (name[0] >= '0' && name[0] <= '9') {
    id = (NodeID)atoi(name);
  } else {
    id = trinity_get_node_id(name);
  }

  if (id > 0) {
    trinity_bind_event(id, 4, node->input_buffer);
  }
  node->ip++;
  return 1;
}

int sys_exec_binding(PufuNodeSystem *sys, PufuNode *node,
                     PufuInstruction *inst) {
  (void)inst;
  NodeID id = (NodeID)node->registers[1];
  char cmd[256];
  if (trinity_get_binding(id, 4, cmd)) {
    pufu_tws_log(node->tws_id, "Event Binding: Spawning %s...", cmd);
    PufuNode *child = pufu_node_load(sys, cmd);
    if (child) {
      child->tws_id = node->tws_id;
    }
  }
  node->ip++;
  return 1;
}

#include "../../system/meow_parser.h" // Include header for parser

int sys_trinity_load_meow(PufuNode *node, PufuInstruction *inst) {
  char filename[256];
  clean_string_arg(filename, inst->reg1);

  // Check if filename is relative or absolute?
  // Trinity Parser reads file using standard C fopen.
  // If we are in pufu root, relative paths work.

  NodeID id = trinity_load_meow(filename);
  if (id > 0) {
    node->registers[0] = id;
  } else {
    node->registers[0] = 0; // Fail
  }

  node->ip++;
  return 1;
}

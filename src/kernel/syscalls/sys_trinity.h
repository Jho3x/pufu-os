#ifndef SYS_TRINITY_H
#define SYS_TRINITY_H

#include "pufu/engine.h"
#include "pufu/node.h"
#include "pufu/syscall_ids.h"

int sys_trinity_init(PufuNode *node);
int sys_trinity_step(PufuNode *node);
int sys_window_init(PufuNode *node, PufuInstruction *inst);
int sys_create_ui_button(PufuNode *node, PufuInstruction *inst);
int sys_trinity_set_vec3(PufuNode *node, PufuInstruction *inst);
int sys_trinity_set_string(PufuNode *node, PufuInstruction *inst);
int sys_create_ui_image(PufuNode *node, PufuInstruction *inst);
int sys_create_ui_window(PufuNode *node, PufuInstruction *inst);
int sys_trinity_set_vec4(PufuNode *node, PufuInstruction *inst);
int sys_trinity_get_vec4(PufuNode *node, PufuInstruction *inst);
int sys_trinity_update_rect(PufuNode *node, PufuInstruction *inst);
int sys_trinity_poll_event(PufuNode *node);
int sys_bind_event(PufuNode *node, PufuInstruction *inst);
int sys_exec_binding(PufuNodeSystem *sys, PufuNode *node,
                     PufuInstruction *inst);

// Meow
int sys_trinity_load_meow(PufuNode *node, PufuInstruction *inst);

#endif // SYS_TRINITY_H

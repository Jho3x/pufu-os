#ifndef SYS_CORE_H
#define SYS_CORE_H

#include "pufu/engine.h"
#include "pufu/node.h" // Defines PufuNode, PufuNodeSystem
#include "pufu/syscall_ids.h"

// Helper for strings
void clean_string_arg(char *dest, const char *src);

int sys_write(PufuNode *node, PufuInstruction *inst);
int sys_read_char(PufuNode *node, PufuInstruction *inst);
int sys_console_input(PufuNode *node, PufuInstruction *inst);
int sys_print_char(PufuNode *node, PufuInstruction *inst);
int sys_clear_buffer(PufuNode *node);
int sys_set_buffer(PufuNode *node, PufuInstruction *inst);
int sys_log_buffer(PufuNode *node);
int sys_string_cmp(PufuNode *node, PufuInstruction *inst);
int sys_console_clear(PufuNode *node);
int sys_set_prompt(PufuNode *node, PufuInstruction *inst);
int sys_cat(PufuNode *node, PufuInstruction *inst);
int sys_config_get(PufuNode *node, PufuInstruction *inst);
int sys_prepend_string(PufuNode *node, PufuInstruction *inst);
int sys_append_string(PufuNode *node, PufuInstruction *inst);
int sys_itoa(PufuNode *node);
int sys_get_version(PufuNode *node);

int sys_core_dispatch(PufuNode *node, PufuInstruction *inst);

#endif // SYS_CORE_H

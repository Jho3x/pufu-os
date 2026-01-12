#ifndef SYS_PROCESS_H
#define SYS_PROCESS_H

#include "pufu/engine.h"
#include "pufu/node.h"
#include "pufu/syscall_ids.h"

int sys_spawn(PufuNodeSystem *sys, PufuNode *node, PufuInstruction *inst);
int sys_exec(PufuNodeSystem *sys, PufuNode *node, PufuInstruction *inst);
int sys_kill(PufuNodeSystem *sys, PufuNode *node, PufuInstruction *inst);
int sys_exit(PufuNode *node);
int sys_shutdown(PufuNode *node);
int sys_sleep(PufuNode *node, PufuInstruction *inst);
int sys_spawn_from_buffer(PufuNodeSystem *sys, PufuNode *node);
int sys_kill_from_buffer(PufuNodeSystem *sys, PufuNode *node);
int sys_parse_command(PufuNode *node, PufuInstruction *inst);

#endif // SYS_PROCESS_H

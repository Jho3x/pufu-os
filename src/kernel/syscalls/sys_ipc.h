#ifndef SYS_IPC_H
#define SYS_IPC_H

#include "pufu/engine.h"
#include "pufu/node.h"
#include "pufu/syscall_ids.h"

int sys_ipc_send(PufuNodeSystem *sys, PufuNode *node, PufuInstruction *inst);
int sys_ipc_read(PufuNode *node, PufuInstruction *inst);
int sys_ipc_broadcast(PufuNodeSystem *sys, PufuNode *node);

#endif // SYS_IPC_H

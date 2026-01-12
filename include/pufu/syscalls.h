#ifndef PUFU_SYSCALLS_H
#define PUFU_SYSCALLS_H

#include "pufu/node.h"

// Dispatch a syscall instruction
// Returns 1 if handled, 0 if not
int pufu_syscall_dispatch(PufuNodeSystem *sys, PufuNode *node,
                          PufuInstruction *inst);

#endif // PUFU_SYSCALLS_H

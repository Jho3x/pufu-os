#include "pufu/engine.h"
#include "pufu/syscall_ids.h"
#include "pufu/trinity.h" // For TWS switch if kept here
#include "syscalls/sys_core.h"
#include "syscalls/sys_ipc.h"
#include "syscalls/sys_process.h"
#include "syscalls/sys_trinity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward decl for TWS if not moved
void pufu_tws_switch(int id);

// Rename back to match node_exec.c expectation
int pufu_syscall_dispatch(PufuNodeSystem *sys, PufuNode *node,
                          PufuInstruction *inst) {
  // Debug trace
  if (inst->syscall_id != SYS_WRITE) { // Ignore write/print_char spam
    // printf("[KERNEL] Syscall Dispatch: ID=%d (Op=%d) Node='%s'\n",
    //        inst->syscall_id, inst->opcode,
    //        node->filename ? node->filename : "unknown");
  }

  // Handle Core Syscalls via sys_core module
  if (sys_core_dispatch(node, inst))
    return 1;

  switch (inst->syscall_id) {
  // Core syscalls handled above.

  // --- CORE IO ---
  // Cases moved to sys_core_dispatch

  // --- PROCESS ---
  case SYS_SPAWN:
    return sys_spawn(sys, node, inst);
  case SYS_EXEC:
    return sys_exec(sys, node, inst);
  case SYS_KILL:
    return sys_kill(sys, node, inst);
  case SYS_EXIT:
    return sys_exit(node);
  case SYS_SHUTDOWN:
    return sys_shutdown(node);
  case SYS_SLEEP:
    return sys_sleep(node, inst);
  case SYS_SPAWN_FROM_BUFFER:
    return sys_spawn_from_buffer(sys, node);
  case SYS_KILL_FROM_BUFFER:
    return sys_kill_from_buffer(sys, node);
  case SYS_PARSE_COMMAND:
    return sys_parse_command(node, inst);

  // --- TRINITY ---
  case SYS_TRINITY_INIT:
    return sys_trinity_init(node);
  case SYS_TRINITY_STEP:
    return sys_trinity_step(node);
  case SYS_WINDOW_INIT:
    return sys_window_init(node, inst);
  case SYS_CREATE_UI_BUTTON:
    return sys_create_ui_button(node, inst);
  case SYS_TRINITY_SET_VEC3:
    return sys_trinity_set_vec3(node, inst);
  case SYS_TRINITY_SET_STRING:
    return sys_trinity_set_string(node, inst);
  case SYS_CREATE_UI_IMAGE:
    return sys_create_ui_image(node, inst);
  case SYS_CREATE_UI_WINDOW:
    return sys_create_ui_window(node, inst);
  case SYS_TRINITY_SET_VEC4:
    return sys_trinity_set_vec4(node, inst);
  case SYS_TRINITY_GET_VEC4:
    return sys_trinity_get_vec4(node, inst);
  case SYS_TRINITY_UPDATE_RECT:
    return sys_trinity_update_rect(node, inst);
  case SYS_TRINITY_POLL_EVENT:
    return sys_trinity_poll_event(node);
  case SYS_BIND_EVENT:
    return sys_bind_event(node, inst);
  case SYS_EXEC_BINDING:
    return sys_exec_binding(sys, node, inst);
  case SYS_TRINITY_LOAD_MEOW:
    return sys_trinity_load_meow(node, inst);

  // --- TWS (Legacy) ---
  case SYS_TWS_SWITCH: {
    // Keep legacy TWS logic here or move to sys_tws.c?
    // User requested split. I'll keep it short here or move if creating
    // sys_tws.c. For 2 calls, let's keep inline.
    int id =
        (strlen(inst->reg1) > 0) ? atoi(inst->reg1) : atoi(node->input_buffer);
    pufu_tws_switch(id);
    node->ip++;
    return 1;
  }
  case SYS_TWS_SWITCH_ARGS:
    pufu_tws_switch(atoi(node->args));
    node->ip++;
    return 1;

  // --- IPC ---
  case SYS_IPC_SEND:
    return sys_ipc_send(sys, node, inst);
  case SYS_IPC_READ:
    return sys_ipc_read(node, inst);
  case SYS_IPC_BROADCAST:
    return sys_ipc_broadcast(sys, node);

  default:
    return 0; // Unknown
  }
}

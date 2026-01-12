# Pufu Kernel

This directory resides at `src/kernel` and contains the privileged core logic of the Operating System. Note that "Kernel" in Pufu is often implemented as a user-space abstraction running on top of the host Linux kernel, but it logically manages Pufu's internal processes and system calls.

## Files

### Sycall Dispatch
*   **`dispatch.c`**: The central hub for handling System Calls (`SYS_*`). It receives requests from the VM and routes them to the appropriate handler.

### System Call Handlers (`src/kernel/syscalls`)
*   **`sys_core.c`**: Handles fundamental OS operations like configuration reading (`SYS_CONFIG_GET`) and string manipulation.
*   **`sys_process.c`**: Manages process lifecycles (`SYS_SPAWN`, `SYS_KILL`, `SYS_EXIT`).
*   **`sys_trinity.c`**: The bridge to the Trinity Graphics subsystem. Handles UI creation (`SYS_TRINITY_CREATE_NODE`), property updates (`SYS_TRINITY_SET_VEC4`), and layout commands.
*   **`sys_ipc.c`**: Handles Inter-Process Communication via the Virtual Bus.

### Inter-Process Communication (Virtual Bus)
The Kernel implements a message-passing system that allows nodes to communicate silently and efficiently.

1.  **Node-to-Node Messaging**:
    *   When a node executes `syscall (ipc_send) "target:message"`, the Kernel (`sys_ipc.c`) locates the target node in memory.
    *   The message is written directly into the target's `ipc_queue` (Mailbox).
    *   The target node reads this queue using `syscall (ipc_read)`.

2.  **Hardware Abstraction**:
    *   Hardware signals are not direct IRQs in User Space.
    *   Instead, "Driver Nodes" (e.g., `keyboard.pufu` or `arm_socket.c`) act as bridges. They read physical inputs and inject standard IPC messages (e.g., `task_manager:KEY_PRESS`) into the bus.

3.  **Roles**:
    *   **Virtual Bus (`system/virtual_bus.pufu`)**: The passive medium. It keeps the IPC subsystem active but does not "control" logic.
    *   **Task Manager**: An active privileged consumer. It listens for specific administrative commands (shutdown, kill) on the bus and executes them.

## Architecture
The Kernel is designed to be modular. When a `.pufu` node executes a `syscall` instruction, the VM pauses execution and invokes `pufu_syscall_dispatch`. This function identifies the syscall ID and delegates execution to a specialized handler in `syscalls/`.

#ifndef PUFU_SYSCALL_IDS_H
#define PUFU_SYSCALL_IDS_H

typedef enum {
  SYS_UNKNOWN = 0,

  // Terminal & IO
  SYS_WRITE = 1,
  SYS_READ_CHAR = 2,
  SYS_CONSOLE_INPUT = 3,
  SYS_PRINT_CHAR = 4,
  SYS_CLEAR_BUFFER = 5,
  SYS_CONSOLE_CLEAR = 6,
  SYS_SET_PROMPT = 7,
  SYS_CAT = 8,
  SYS_LOG_BUFFER = 9,
  SYS_PREPEND_STRING = 10,
  SYS_APPEND_STRING = 11,
  SYS_SET_BUFFER = 12,

  // Process/Task
  SYS_SPAWN = 20,
  SYS_EXEC = 21,
  SYS_KILL = 22,
  SYS_EXIT = 23,
  SYS_SLEEP = 24,
  SYS_SHUTDOWN = 25,
  SYS_SPAWN_FROM_BUFFER = 26,
  SYS_KILL_FROM_BUFFER = 27,
  SYS_PARSE_COMMAND = 28,

  // IPC
  SYS_IPC_SEND = 30, // (ipc_send_from_buffer)
  SYS_IPC_READ = 31,
  SYS_IPC_BROADCAST = 32,

  // Config
  SYS_CONFIG_GET = 40,

  // TWS
  SYS_TWS_SWITCH = 50,
  SYS_TWS_SWITCH_ARGS = 51,

  // Trinity / Graphics
  SYS_TRINITY_INIT = 60,
  SYS_TRINITY_STEP = 61,
  SYS_WINDOW_INIT = 62,
  SYS_STRING_CMP = 63,
  SYS_WINDOW_CLEAR = 64,
  SYS_WINDOW_SWAP = 65,
  SYS_WINDOW_DRAW_MODEL = 66,

  // UI
  SYS_CREATE_UI_BUTTON = 70,    // Create UI Node
  SYS_TRINITY_SET_VEC3 = 71,    // Set Vec3 Property
  SYS_TRINITY_SET_STRING = 72,  // Set String Property
  SYS_CREATE_UI_IMAGE = 73,     // Create UI Image
  SYS_CREATE_UI_WINDOW = 74,    // Create UI Window
  SYS_TRINITY_POLL_EVENT = 75,  // Poll Input Event
  SYS_TRINITY_SET_VEC4 = 76,    // Set Vec4 Property
  SYS_TRINITY_GET_VEC4 = 77,    // Get Vec4 Property
  SYS_TRINITY_UPDATE_RECT = 78, // Set Rect from Regs
  SYS_ITOA = 79,                // Int to String (InputBuffer)
  SYS_BIND_EVENT = 80,          // Bind Event to Command
  SYS_EXEC_BINDING = 81,        // Execute Bound Command
  SYS_GET_VERSION = 82,         // Get Pufu Version String
  SYS_TRINITY_LOAD_MEOW = 83,   // Load Meow UI File
  SYS_SYSTEM_UPDATE = 99,       // Hot Swap Update
  SYS_DOWNLOAD_UPDATE = 100

} PufuSyscallID;

#endif // PUFU_SYSCALL_IDS_H

#define _POSIX_C_SOURCE 200809L
#include "pufu/socket.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

// Mensajes de bienvenida
static const char *welcome_messages[] = {
    "                              x", "    ▓         ▓               0",
    "    ▓ ▓     ▓ ▓               1", "  ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓     ▓ ▓ ▓   2",
    "  ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓   ▓ ▓ ▓ ▓ ▓ 3", "  ▓     ▓ ▓     ▓   ▓ ▓   ▓ ▓ 4",
    "  ▓     ▓ ▓     ▓   ▓ ▓       5", "  ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓   ▓ ▓ ▓ ▓   6",
    "  ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓     ▓ ▓ ▓   7", "        ▓ ▓             ▓ ▓   8",
    "      ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓   9", "      ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓     A",
    "      ▓     ▓   ▓ ▓   ▓ ▓     B", "      ▓     ▓   ▓ ▓   ▓ ▓     C",
    "      ▓     ▓     ▓     ▓     D", "                              E",
    "            PUFU!             F", "Pardum Felidum Operating System"};

// Implementación de las funciones del socket ARM
static int arm_init(void) {
  printf("Inicializando socket ARM...\n");
  return 0;
}

static int arm_load_file(const char *filename) {
  printf("Cargando archivo en ARM: %s\n", filename);
  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("Error: No se pudo abrir el archivo %s\n", filename);
    return -1;
  }
  char buffer[1024];
  while (fgets(buffer, sizeof(buffer), file)) {
    printf("%s", buffer);
  }
  fclose(file);
  return 0;
  return 0;
}

// --- ALU Implementation ---
static int arm_alu_add(int a, int b) { return a + b; }
static int arm_alu_sub(int a, int b) { return a - b; }
static int arm_alu_mul(int a, int b) { return a * b; }
static int arm_alu_div(int a, int b) { return (b != 0) ? (a / b) : 0; }
static int arm_alu_cmp(int a, int b) {
  if (a < b)
    return -1;
  if (a > b)
    return 1;
  return 0;
}

// Helper for non-blocking key check
static int kbhit(void) {
  // Rely on core terminal management
  // Note: This consumes the key if present!
  // Forward declare or include "pufu/terminal.h"
  extern int pufu_terminal_get_key(void);
  return pufu_terminal_get_key();
}

// Función para manejar syscalls especiales
static int handle_special_syscall(const char *syscall_name,
                                  const char *string_arg, int value) {
  // printf("Handle Special: %s String: %s\n", syscall_name, string_arg);
  // fflush(stdout);
  if (strcmp(syscall_name, "(write)") == 0) {
    if (string_arg && string_arg[0]) {
      printf("%s\n", string_arg);
      return 0;
    } else {
      // Usar value como índice para mensajes predefinidos
      if (value < 0 || value >= (int)(sizeof(welcome_messages) /
                                      sizeof(welcome_messages[0]))) {
        return -1;
      }
      printf("%s\n", welcome_messages[value]);
      return 0;
    }
  } else if (strcmp(syscall_name, "(stats)") == 0) {
    // Simulated System Stats
    int cpu = rand() % 20 + 5;     // 5-25%
    int ram = 128 + (rand() % 16); // 128-144 MB
    int nodes = 1;                 // Currently running node
    // Uptime simulation
    static time_t start_time = 0;
    if (start_time == 0)
      start_time = time(NULL);
    int uptime = (int)(time(NULL) - start_time);

    printf("\r[CPU: %2d%%] [RAM: %3d MB] [NODES: %d] [UPTIME: %3ds] (Press "
           "Ctrl+C to exit)",
           cpu, ram, nodes, uptime);
    fflush(stdout);
    return 0;
  } else if (strcmp(syscall_name, "(sleep)") == 0) {
    // Sleep for ms
    int ms = 0;
    if (string_arg && string_arg[0]) {
      ms = atoi(string_arg);
    } else {
      // Try parsing "ms=X" if string_arg is null or empty?
      // The parser passes "ms=1000" as string_arg if written as `syscall
      // (sleep) "1000"` But if written as `syscall (sleep) 1000`, it might be
      // in reg1? Let's assume string_arg contains the number string.
    }
    if (ms > 0)
      usleep(ms * 1000);
    return 0;
  } else if (strcmp(syscall_name, "(exit_if_key)") == 0) {
    int key = kbhit();
    if (key > 0) {
      char target = string_arg ? string_arg[0] : 'q';
      if (key == target) {
        printf("\nExiting...\n");
        exit(0);
      }
    }
    return 0;
  } else if (strcmp(syscall_name, "(exit)") == 0) {
    exit(0); // Terminar el proceso correctamente
  } else if (strcmp(syscall_name, "(load_file)") == 0 ||
             strcmp(syscall_name, "(read_file)") == 0) {
    if (string_arg && string_arg[0]) {
      return arm_load_file(string_arg);
    }
    // Fallback legacy behavior
    if (strcmp(syscall_name, "(load_file)") == 0)
      return arm_load_file("char.pufu");
    if (strcmp(syscall_name, "(read_file)") == 0)
      return arm_load_file("welcome.pufu");
  }
  return -1;
}

static int arm_execute(const char *code) {
  // printf("ARM Execute: %s\n", code);
  // fflush(stdout);
  // Parsear la instrucción manualmente para soportar strings
  char op[32];
  char arg1[64];
  char string_arg[256] = {0};
  int value = -1;

  // Simple parser
  const char *ptr = code;
  while (*ptr && *ptr == ' ')
    ptr++;

  // Op
  int i = 0;
  while (*ptr && *ptr != ' ' && i < 31)
    op[i++] = *ptr++;
  op[i] = 0;

  if (strcmp(op, "syscall") == 0) {
    while (*ptr && *ptr == ' ')
      ptr++;

    // Arg1 (syscall name)
    i = 0;
    while (*ptr && *ptr != ' ' && i < 63)
      arg1[i++] = *ptr++;
    arg1[i] = 0;

    // Check for string argument
    while (*ptr && *ptr == ' ')
      ptr++;
    if (*ptr == '"') {
      ptr++; // Skip quote
      i = 0;
      while (*ptr && *ptr != '"' && i < 255)
        string_arg[i++] = *ptr++;
      string_arg[i] = 0;
    } else if (strncmp(ptr, "num=", 4) == 0) {
      value = atoi(ptr + 4);
    }

    // Manejar syscalls especiales
    return handle_special_syscall(arg1, string_arg, value);
  } else if (strcmp(op, "mul") == 0 || strcmp(op, "add") == 0 ||
             strcmp(op, "sub") == 0 || strcmp(op, "div") == 0) {
    // Simple arithmetic simulation: op val1 val2
    // In a real CPU this would operate on registers.
    // Here we parse two integers and print the result.
    while (*ptr && *ptr == ' ')
      ptr++;
    int v1 = atoi(ptr);
    while (*ptr && *ptr != ' ')
      ptr++;
    while (*ptr && *ptr == ' ')
      ptr++;
    int v2 = atoi(ptr);

    if (strcmp(op, "mul") == 0) {
      printf("Paw MUL: %d * %d = %d\n", v1, v2, v1 * v2);
    } else if (strcmp(op, "add") == 0) {
      printf("Paw ADD: %d + %d = %d\n", v1, v2, v1 + v2);
    } else if (strcmp(op, "sub") == 0) {
      printf("Paw SUB: %d - %d = %d\n", v1, v2, v1 - v2);
    } else if (strcmp(op, "div") == 0) {
      if (v2 == 0) {
        printf("Paw DIV: Error (Division by zero)\n");
        return -1;
      }
      printf("Paw DIV: %d / %d = %d\n", v1, v2, v1 / v2);
    }
    return 0;
  }

  return 0;
}

static const char *arm_get_arch_name(void) { return "ARM"; }

static int arm_get_word_size(void) {
  return 32; // ARM típicamente usa 32 bits
}

static void arm_cleanup(void) {
  printf("Limpiando recursos del socket ARM...\n");
}

// Instancia del socket ARM
static PufuSocket arm_socket = {.init = arm_init,
                                .execute = arm_execute,
                                .load_file = arm_load_file,
                                .get_arch_name = arm_get_arch_name,
                                .get_word_size = arm_get_word_size,
                                .alu_add = arm_alu_add,
                                .alu_sub = arm_alu_sub,
                                .alu_mul = arm_alu_mul,
                                .alu_div = arm_alu_div,
                                .alu_cmp = arm_alu_cmp,
                                .cleanup = arm_cleanup,
                                .api_version = PUFU_SOCKET_API_VERSION,
                                .syscall = NULL,
                                .get_state_size = NULL,
                                .save_state = NULL,
                                .restore_state = NULL};

// -- Simple State Persistence Implementation --
static int socket_counter = 0;

typedef struct {
  int counter;
} ArmState;

static size_t arm_get_state_size(void) { return sizeof(ArmState); }

static int arm_save_state(void *buffer, size_t size) {
  if (size < sizeof(ArmState))
    return -1;
  ArmState *s = (ArmState *)buffer;
  s->counter = socket_counter;
  printf("[ARM-V1] State Saved: Counter=%d\n", s->counter);
  return 0;
}

static int arm_restore_state(const void *buffer, size_t size) {
  if (size < sizeof(ArmState))
    return -1;
  const ArmState *s = (const ArmState *)buffer;
  socket_counter = s->counter;
  printf("[ARM-V1] State Restored: Counter=%d\n", socket_counter);
  return 0;
}

static int arm_syscall(int id, void *args) {
  (void)id;
  (void)args;
  socket_counter++; // Increment on syscall
  return 0;
}

// Update struct with new methods
static void update_struct(void) __attribute__((constructor));
static void update_struct(void) {
  arm_socket.get_state_size = arm_get_state_size;
  arm_socket.save_state = arm_save_state;
  arm_socket.restore_state = arm_restore_state;
  arm_socket.syscall = arm_syscall;
}

// Función para obtener el socket ARM
PufuSocket *pufu_get_arm_socket(void) { return &arm_socket; }
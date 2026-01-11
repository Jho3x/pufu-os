#define _POSIX_C_SOURCE 199309L // For usleep
#include "pufu/dyn_loader.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// --- V2 Implementation (Mock Network) ---

// ALU Operations (Networked / Cloud)
int alu_add_net(int a, int b) {
  printf("[NET-V2] Cloud Add: %d + %d\n", a, b);
  return a + b;
}

int alu_sub_net(int a, int b) {
  printf("[NET-V2] Cloud Sub: %d - %d\n", a, b);
  return a - b;
}

int alu_mul_net(int a, int b) { return a * b; }

int alu_div_net(int a, int b) {
  if (b == 0)
    return 0;
  return a / b;
}

// Missing in header but good to implement to match struct order if needed, or
// rely on designated initializers
int alu_cmp_net(int a, int b) { return (a == b) ? 0 : (a < b ? -1 : 1); }

int socket_init_net(void) {
  printf("\n=== [V2] PufuNet Socket Initialized ===\n");
  printf(">> Connected to Pufu Cloud (Mock)\n");
  printf(">> Latency: 5ms\n");
  return 0;
}

void socket_cleanup_net(void) { printf("[V2] PufuNet Socket Disconnected.\n"); }

/*
    // Inicializar el socket
    int (*init)(void);
    // Ejecutar código Pufu - Assuming NULL as we are a slave socket usually? or
   copy from V1? int (*execute)(const char* code);
    // Cargar un archivo Pufu
    int (*load_file)(const char* filename);
    // Obtener el nombre de la arquitectura
    const char* (*get_arch_name)(void);
    // Obtener el tamaño de palabra
    int (*get_word_size)(void);
    // ALU Operations
    int (*alu_add)(int a, int b);
    int (*alu_sub)(int a, int b);
    int (*alu_mul)(int a, int b);
    int (*alu_div)(int a, int b);
    int (*alu_cmp)(int a, int b);
    // Limpiar recursos
    void (*cleanup)(void);
*/

const char *get_arch_name_net(void) { return "ARM Cortex-A72 (Networked)"; }
int get_word_size_net(void) { return 32; }
int execute_net(const char *code) {
  (void)code;
  return 0;
}
int load_file_net(const char *filename) {
  (void)filename;
  return 0;
}

// Static instance
static PufuSocket v2_socket = {.init = socket_init_net,
                               .execute = execute_net,
                               .load_file = load_file_net,
                               .get_arch_name = get_arch_name_net,
                               .get_word_size = get_word_size_net,
                               .alu_add = alu_add_net,
                               .alu_sub = alu_sub_net,
                               .alu_mul = alu_mul_net,
                               .alu_div = alu_div_net,
                               .alu_cmp = alu_cmp_net,
                               .cleanup = socket_cleanup_net,
                               .api_version = PUFU_SOCKET_API_VERSION,
                               .syscall = NULL,
                               .get_state_size = NULL,
                               .save_state = NULL,
                               .restore_state = NULL};

// -- V2 State Persistence --
static int socket_counter = -1; // New default

typedef struct {
  int counter;
} ArmState;

static size_t net_get_state_size(void) { return sizeof(ArmState); }

static int net_save_state(void *buffer, size_t size) {
  if (size < sizeof(ArmState))
    return -1;
  ArmState *s = (ArmState *)buffer;
  s->counter = socket_counter;
  printf("[NET-V2] State Saved: Counter=%d\n", s->counter);
  return 0;
}

static int net_restore_state(const void *buffer, size_t size) {
  if (size < sizeof(ArmState))
    return -1;
  const ArmState *s = (const ArmState *)buffer;
  socket_counter = s->counter;
  printf("[NET-V2] State Restored: Counter=%d\n", socket_counter);
  return 0;
}

static int net_syscall(int id, void *args) {
  (void)id;
  (void)args;
  socket_counter++;
  printf("[NET-V2] Syscall Executed. Counter now: %d\n", socket_counter);
  return 0;
}

// Update struct with new methods
static void update_struct(void) __attribute__((constructor));
static void update_struct(void) {
  v2_socket.get_state_size = net_get_state_size;
  v2_socket.save_state = net_save_state;
  v2_socket.restore_state = net_restore_state;
  v2_socket.syscall = net_syscall;
}

// Exported Entry Point
PufuSocket *pufu_get_arm_socket(void) { return &v2_socket; }

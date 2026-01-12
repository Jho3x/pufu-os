#ifndef PUFU_SOCKET_H
#define PUFU_SOCKET_H

#include <stddef.h>
#include <stdint.h>

// Estructura para la interfaz común de sockets
typedef struct {
  // Inicializar el socket
  int (*init)(void);

  // Ejecutar código Pufu
  int (*execute)(const char *code);

  // Cargar un archivo Pufu
  int (*load_file)(const char *filename);

  // Obtener el nombre de la arquitectura
  const char *(*get_arch_name)(void);

  // Obtener el tamaño de palabra
  int (*get_word_size)(void);

  // ALU Operations (Hardware Abstraction)
  int (*alu_add)(int a, int b);
  int (*alu_sub)(int a, int b);
  int (*alu_mul)(int a, int b);
  int (*alu_div)(int a, int b);
  int (*alu_cmp)(int a, int b);

  // Limpiar recursos del socket
  void (*cleanup)(void);

  // --- New Features (v2) ---
  uint32_t api_version;

  // Standardized Syscall Interface
  int (*syscall)(int id, void *args);

  // State Persistence (Hot Swap)
  size_t (*get_state_size)(void);
  int (*save_state)(void *buffer, size_t size);
  int (*restore_state)(const void *buffer, size_t size);

} PufuSocket;

#define PUFU_SOCKET_API_VERSION 1

// Función para obtener el socket actual
PufuSocket *pufu_get_current_socket(void);

// Función para establecer el socket actual
void pufu_set_current_socket(PufuSocket *socket);

// Función para obtener el socket ARM
PufuSocket *pufu_get_arm_socket(void);

#endif // PUFU_SOCKET_H
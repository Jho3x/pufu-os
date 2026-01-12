#ifndef PUFU_NODE_H
#define PUFU_NODE_H

#include "pufu/hot_reload.h"
#include "pufu/parser.h"

#include "pufu/crystal.h"
#include "pufu/virtual_bus.h"

// Tipos de nodos
typedef enum {
  PUFU_NODE_ASSEMBLER,    // Nodo clásico (.pufu)
  PUFU_NODE_CRYSTAL,      // Nodo Soft-FPGA (.crystal)
  PUFU_NODE_TRINITY_SCENE // Escena gráfica (auto-launch Trinity)
} PufuNodeType;

// Forward declaration
struct PufuEntity;

// Estructura para mensajes IPC
typedef struct {
  char sender[64];
  char content[192];
  int type; // 0=Text, 1=ProcessExit, 2=Broadcast
} PufuMessage;

#define PUFU_IPC_QUEUE_SIZE 16

// Estructura para representar un nodo Pufu
typedef struct PufuNode {
  char *filename;    // Nombre del archivo
  PufuNodeType type; // Tipo de nodo
  struct PufuEntity
      *body; // The Physical Body (Trinity Entity) linked to this Logic Node
  PufuParser *parser;     // Parser del nodo (para Assembler)
  PufuCrystal *crystal;   // Motor Crystal (para Soft-FPGA)
  PufuHotReload *reload;  // Sistema de hot-reload
  struct PufuNode *next;  // Siguiente nodo en la lista
  int is_arbiter;         // Si es el nodo árbitro
  int loop_counter;       // Contador para bucles (restart)
  int ip;                 // Instruction Pointer (para multitasking)
  int active;             // Si el nodo está activo/ejecutándose
  long long wake_time;    // Tiempo (ms) para despertar (si está durmiendo)
  int registers[16];      // Registros de propósito general (r0-r15)
  int cmp_flag;           // Flag de comparación (-1, 0, 1)
  char input_buffer[256]; // Buffer de entrada para terminal
  int input_pos;          // Posición actual en el buffer

  // IPC Event Bus (Queue)
  PufuMessage ipc_queue[PUFU_IPC_QUEUE_SIZE];
  int ipc_head;  // Write Index
  int ipc_tail;  // Read Index
  int ipc_count; // Messages pending

  char args[256]; // Argumentos de lanzamiento (e.g. "NO_GUI")
  int tws_id;     // Terminal Window Space ID (0 by default)
} PufuNode;

// Estructura para el sistema de nodos
typedef struct {
  PufuNode *arbiter;   // Nodo árbitro actual
  PufuNode *nodes;     // Lista de nodos cargados
  PufuVirtualBus *bus; // Bus de mensajes IPC (VirtualBus)
} PufuNodeSystem;

// Inicializar el sistema de nodos
PufuNodeSystem *pufu_node_system_init(void);

// Cargar un nodo
PufuNode *pufu_node_load(PufuNodeSystem *system, const char *filename);

// Establecer un nodo como árbitro
int pufu_node_set_arbiter(PufuNodeSystem *system, PufuNode *node);

// Ejecutar un nodo (un paso)
int pufu_node_execute(PufuNodeSystem *system, PufuNode *node);

// Verificar cambios en todos los nodos
// Verificar cambios en todos los nodos
int pufu_node_system_check(PufuNodeSystem *system);

// VM Helpers
int get_reg_index(const char *str);
int get_value(PufuNode *node, const char *str);
long long get_time_ms(void);
int find_label(PufuNode *node, const char *label);

// Node Lifecycle (Internal/Core)
PufuNode *pufu_create_node(const char *filename, int type_override);

// Liberar recursos del sistema
// Liberar recursos del sistema
void pufu_node_system_cleanup(PufuNodeSystem *system);

// Global Terminal Setup
void pufu_terminal_init(void);
void pufu_terminal_restore(void);
void pufu_log(const char *fmt, ...);
void pufu_os_shutdown(void);

#endif // PUFU_NODE_H
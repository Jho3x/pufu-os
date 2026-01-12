#ifndef PUFU_PARSER_H
#define PUFU_PARSER_H

#include "pufu/opcode.h"
#include <stdint.h>

// Estructura para representar una instrucción Pufu
typedef struct {
  PufuOpcode opcode; // Numeric Opcode (The Optimization!)
  char op[32];       // Legacy string op (kept for debug)
  char reg1[128];    // Primer registro (reg.1, reg.2, etc.)
  char reg2[128];    // Segundo registro (si es necesario)
  char value[128];   // Valor o dirección
  int is_syscall;    // Flag para syscalls
  int syscall_id;    // Optimized Syscall ID
} PufuInstruction;

// Label Entry for O(1) Jumps
typedef struct {
  char name[64];
  int index;
} PufuLabelEntry;

// Estructura para el parser
typedef struct {
  PufuInstruction *instructions; // Array de instrucciones
  int count;                     // Número de instrucciones
  int capacity;                  // Capacidad actual del array
  int has_pufu_init;             // Flag 3.0: Archivo inicia con _pufu::init

  // Label Index
  PufuLabelEntry *labels;
  int label_count;
  int label_capacity;
} PufuParser;

// Inicializar el parser
PufuParser *pufu_parser_init(void);

// Parsear una línea de código Pufu
int pufu_parse_line(PufuParser *parser, const char *line);

// Parsear un archivo Pufu completo
int pufu_parse_file(PufuParser *parser, const char *filename);

// Liberar recursos del parser
void pufu_parser_cleanup(PufuParser *parser);

#endif // PUFU_PARSER_H
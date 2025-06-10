#ifndef PUFU_PARSER_H
#define PUFU_PARSER_H

#include <stdint.h>

// Estructura para representar una instrucción Pufu
typedef struct {
    char op[32];        // Operación (load, store, add, etc.)
    char reg1[32];      // Primer registro (reg.1, reg.2, etc.)
    char reg2[32];      // Segundo registro (si es necesario)
    char value[32];     // Valor o dirección
    int is_syscall;     // Flag para syscalls
} PufuInstruction;

// Estructura para el parser
typedef struct {
    PufuInstruction* instructions;  // Array de instrucciones
    int count;                      // Número de instrucciones
    int capacity;                   // Capacidad actual del array
} PufuParser;

// Inicializar el parser
PufuParser* pufu_parser_init(void);

// Parsear una línea de código Pufu
int pufu_parse_line(PufuParser* parser, const char* line);

// Parsear un archivo Pufu completo
int pufu_parse_file(PufuParser* parser, const char* filename);

// Liberar recursos del parser
void pufu_parser_cleanup(PufuParser* parser);

#endif // PUFU_PARSER_H 
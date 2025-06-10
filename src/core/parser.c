#include "pufu/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_CAPACITY 32

// Inicializar el parser
PufuParser* pufu_parser_init(void) {
    PufuParser* parser = malloc(sizeof(PufuParser));
    if (!parser) return NULL;

    parser->instructions = malloc(sizeof(PufuInstruction) * INITIAL_CAPACITY);
    if (!parser->instructions) {
        free(parser);
        return NULL;
    }

    parser->count = 0;
    parser->capacity = INITIAL_CAPACITY;
    return parser;
}

// Verificar si una línea es un comentario o está vacía
static int is_comment_or_empty(const char* line) {
    while (*line) {
        if (*line == '#') return 1;
        if (!isspace(*line)) return 0;
        line++;
    }
    return 1;
}

// Verificar si una línea es una etiqueta
static int is_label(const char* line) {
    while (*line) {
        if (*line == ':') return 1;
        if (isspace(*line)) return 0;
        line++;
    }
    return 0;
}

// Extraer el valor numérico de un argumento con formato num=X
static int extract_numeric_value(const char* arg) {
    if (strncmp(arg, "num=", 4) == 0) {
        return atoi(arg + 4);
    }
    return atoi(arg);
}

// Parsear una línea de código Pufu
int pufu_parse_line(PufuParser* parser, const char* line) {
    // Ignorar comentarios y líneas vacías
    if (is_comment_or_empty(line)) return 0;
    
    // Ignorar etiquetas por ahora
    if (is_label(line)) return 0;

    // Verificar si necesitamos más espacio
    if (parser->count >= parser->capacity) {
        int new_capacity = parser->capacity * 2;
        PufuInstruction* new_instructions = realloc(parser->instructions, 
                                                  sizeof(PufuInstruction) * new_capacity);
        if (!new_instructions) return -1;
        
        parser->instructions = new_instructions;
        parser->capacity = new_capacity;
    }

    // Inicializar la nueva instrucción
    PufuInstruction* inst = &parser->instructions[parser->count];
    memset(inst, 0, sizeof(PufuInstruction));
    inst->is_syscall = 0;

    // Parsear la línea
    char op[32], arg1[32], arg2[32], arg3[32];
    int count = sscanf(line, "%31s %31s %31s %31s", op, arg1, arg2, arg3);

    if (count < 2) return -1;  // Mínimo necesitamos operación y un argumento

    // Copiar la operación
    strncpy(inst->op, op, sizeof(inst->op) - 1);

    // Verificar si es un syscall
    if (strcmp(op, "syscall") == 0) {
        inst->is_syscall = 1;
        if (arg1[0] == '(' && arg1[strlen(arg1)-1] == ')') {
            // Es un syscall con nombre descriptivo
            strncpy(inst->value, arg1, sizeof(inst->value) - 1);
        } else {
            // Es un syscall con número, puede ser num=X o directamente el número
            int value = extract_numeric_value(arg1);
            snprintf(inst->value, sizeof(inst->value), "%d", value);
        }
    } else {
        // Es una instrucción normal
        strncpy(inst->reg1, arg1, sizeof(inst->reg1) - 1);
        
        if (count > 2) {
            if (strncmp(arg2, "reg.", 4) == 0) {
                // Segundo argumento es un registro
                strncpy(inst->reg2, arg2, sizeof(inst->reg2) - 1);
            } else {
                // Segundo argumento es un valor, puede ser num=X o directamente el valor
                int value = extract_numeric_value(arg2);
                snprintf(inst->value, sizeof(inst->value), "%d", value);
            }
        }
    }

    parser->count++;
    return 0;
}

// Parsear un archivo Pufu completo
int pufu_parse_file(PufuParser* parser, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remover el salto de línea
        line[strcspn(line, "\n")] = 0;
        
        if (pufu_parse_line(parser, line) < 0) {
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}

// Liberar recursos del parser
void pufu_parser_cleanup(PufuParser* parser) {
    if (parser) {
        free(parser->instructions);
        free(parser);
    }
} 
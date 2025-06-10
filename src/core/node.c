#include "pufu/node.h"
#include "pufu/socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Inicializar el sistema de nodos
PufuNodeSystem* pufu_node_system_init(void) {
    PufuNodeSystem* system = malloc(sizeof(PufuNodeSystem));
    if (!system) return NULL;

    system->arbiter = NULL;
    system->nodes = NULL;
    return system;
}

// Crear un nuevo nodo
static PufuNode* create_node(const char* filename) {
    PufuNode* node = malloc(sizeof(PufuNode));
    if (!node) return NULL;

    node->filename = strdup(filename);
    if (!node->filename) {
        free(node);
        return NULL;
    }

    node->parser = pufu_parser_init();
    if (!node->parser) {
        free(node->filename);
        free(node);
        return NULL;
    }

    node->reload = pufu_hot_reload_init(filename);
    if (!node->reload) {
        pufu_parser_cleanup(node->parser);
        free(node->filename);
        free(node);
        return NULL;
    }

    node->next = NULL;
    node->is_arbiter = 0;
    return node;
}

// Cargar un nodo
PufuNode* pufu_node_load(PufuNodeSystem* system, const char* filename) {
    if (!system || !filename) return NULL;

    // Verificar si el nodo ya está cargado
    PufuNode* current = system->nodes;
    while (current) {
        if (strcmp(current->filename, filename) == 0) {
            return current;
        }
        current = current->next;
    }

    // Crear nuevo nodo
    PufuNode* node = create_node(filename);
    if (!node) return NULL;

    // Parsear el archivo
    if (pufu_parse_file(node->parser, filename) < 0) {
        pufu_hot_reload_cleanup(node->reload);
        pufu_parser_cleanup(node->parser);
        free(node->filename);
        free(node);
        return NULL;
    }

    // Iniciar hot-reload
    if (pufu_hot_reload_start(node->reload) < 0) {
        pufu_hot_reload_cleanup(node->reload);
        pufu_parser_cleanup(node->parser);
        free(node->filename);
        free(node);
        return NULL;
    }

    // Agregar a la lista
    node->next = system->nodes;
    system->nodes = node;

    return node;
}

// Establecer un nodo como árbitro
int pufu_node_set_arbiter(PufuNodeSystem* system, PufuNode* node) {
    if (!system || !node) return -1;

    // Desactivar el árbitro actual si existe
    if (system->arbiter) {
        system->arbiter->is_arbiter = 0;
    }

    // Establecer nuevo árbitro
    system->arbiter = node;
    node->is_arbiter = 1;

    return 0;
}

// Ejecutar un nodo
int pufu_node_execute(PufuNode* node) {
    if (!node || !node->parser) return -1;

    printf("Ejecutando nodo: %s\n", node->filename);

    // Obtener el socket ARM
    PufuSocket* socket = pufu_get_arm_socket();
    if (!socket) {
        printf("Error: No se pudo obtener el socket ARM\n");
        return -1;
    }

    // Inicializar el socket
    if (socket->init() < 0) {
        printf("Error: No se pudo inicializar el socket ARM\n");
        return -1;
    }

    // Ejecutar cada instrucción
    for (int i = 0; i < node->parser->count; i++) {
        PufuInstruction* inst = &node->parser->instructions[i];
        
        // Construir la línea de código
        char code[256];
        if (inst->is_syscall) {
            snprintf(code, sizeof(code), "syscall %s", inst->value);
        } else {
            snprintf(code, sizeof(code), "%s %s", inst->op, inst->reg1);
            if (inst->reg2[0]) {
                strncat(code, " ", sizeof(code) - strlen(code) - 1);
                strncat(code, inst->reg2, sizeof(code) - strlen(code) - 1);
            }
            if (inst->value[0]) {
                strncat(code, " ", sizeof(code) - strlen(code) - 1);
                strncat(code, inst->value, sizeof(code) - strlen(code) - 1);
            }
        }

        // Ejecutar la instrucción
        if (socket->execute(code) < 0) {
            printf("Error: No se pudo ejecutar la instrucción: %s\n", code);
            socket->cleanup();
            return -1;
        }
    }

    // Limpiar el socket
    socket->cleanup();
    return 0;
}

// Verificar cambios en todos los nodos
int pufu_node_system_check(PufuNodeSystem* system) {
    if (!system) return -1;

    int changes = 0;
    PufuNode* current = system->nodes;
    while (current) {
        int result = pufu_hot_reload_check(current->reload);
        if (result > 0) {
            printf("\nNodo modificado: %s\n", current->filename);
            if (current->is_arbiter) {
                printf("(Este es el nodo árbitro)\n");
            }
            changes++;
        }
        current = current->next;
    }

    return changes;
}

// Liberar recursos del sistema
void pufu_node_system_cleanup(PufuNodeSystem* system) {
    if (!system) return;

    PufuNode* current = system->nodes;
    while (current) {
        PufuNode* next = current->next;
        pufu_hot_reload_cleanup(current->reload);
        pufu_parser_cleanup(current->parser);
        free(current->filename);
        free(current);
        current = next;
    }

    free(system);
} 
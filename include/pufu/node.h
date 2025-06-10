#ifndef PUFU_NODE_H
#define PUFU_NODE_H

#include "pufu/parser.h"
#include "pufu/hot_reload.h"

// Estructura para representar un nodo Pufu
typedef struct PufuNode {
    char* filename;           // Nombre del archivo
    PufuParser* parser;       // Parser del nodo
    PufuHotReload* reload;    // Sistema de hot-reload
    struct PufuNode* next;    // Siguiente nodo en la lista
    int is_arbiter;          // Si es el nodo árbitro
} PufuNode;

// Estructura para el sistema de nodos
typedef struct {
    PufuNode* arbiter;        // Nodo árbitro actual
    PufuNode* nodes;          // Lista de nodos cargados
} PufuNodeSystem;

// Inicializar el sistema de nodos
PufuNodeSystem* pufu_node_system_init(void);

// Cargar un nodo
PufuNode* pufu_node_load(PufuNodeSystem* system, const char* filename);

// Establecer un nodo como árbitro
int pufu_node_set_arbiter(PufuNodeSystem* system, PufuNode* node);

// Ejecutar un nodo
int pufu_node_execute(PufuNode* node);

// Verificar cambios en todos los nodos
int pufu_node_system_check(PufuNodeSystem* system);

// Liberar recursos del sistema
void pufu_node_system_cleanup(PufuNodeSystem* system);

#endif // PUFU_NODE_H 
#include "pufu/node.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static int running = 1;

void handle_signal(int sig) {
    running = 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: %s <so.pufu> [nodo1.pufu] [nodo2.pufu] ...\n", argv[0]);
        return 1;
    }

    // Configurar manejo de señales
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Inicializar sistema de nodos
    PufuNodeSystem* system = pufu_node_system_init();
    if (!system) {
        printf("Error al inicializar sistema de nodos\n");
        return 1;
    }

    // Cargar nodo árbitro
    PufuNode* arbiter = pufu_node_load(system, argv[1]);
    if (!arbiter) {
        printf("Error al cargar nodo árbitro: %s\n", argv[1]);
        pufu_node_system_cleanup(system);
        return 1;
    }

    // Establecer como árbitro
    if (pufu_node_set_arbiter(system, arbiter) < 0) {
        printf("Error al establecer nodo árbitro\n");
        pufu_node_system_cleanup(system);
        return 1;
    }

    // Cargar nodos adicionales
    for (int i = 2; i < argc; i++) {
        PufuNode* node = pufu_node_load(system, argv[i]);
        if (!node) {
            printf("Error al cargar nodo: %s\n", argv[i]);
            continue;
        }
        printf("Nodo cargado: %s\n", argv[i]);
    }

    printf("\nSistema de nodos iniciado\n");
    printf("Nodo árbitro: %s\n", arbiter->filename);
    printf("Presiona Ctrl+C para salir\n\n");

    // Ejecutar el nodo árbitro al inicio para mostrar el mensaje de bienvenida
    pufu_node_execute(system->arbiter);

    // Bucle principal
    while (running) {
        // Verificar cambios en todos los nodos
        int changes = pufu_node_system_check(system);
        if (changes > 0) {
            printf("\nEjecutando nodo árbitro...\n");
            pufu_node_execute(system->arbiter);
        }

        // Pequeña pausa
        usleep(100000);  // 100ms
    }

    printf("\nCerrando sistema de nodos...\n");
    pufu_node_system_cleanup(system);
    return 0;
} 
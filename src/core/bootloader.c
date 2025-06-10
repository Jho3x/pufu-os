#include "pufu/node.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static int running = 1;

void handle_signal(int sig) {
    (void)sig;  // Evitar advertencia de parámetro no usado
    running = 0;
}

// Función para mostrar una animación de carga
void show_loading_animation(void) {
    const char* frames[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
    const int num_frames = sizeof(frames) / sizeof(frames[0]);
    
    printf("\n");
    for (int i = 0; i < 20; i++) {
        printf("\r%s Cargando nodo inicial...", frames[i % num_frames]);
        fflush(stdout);
        usleep(100000);  // 100ms
    }
    printf("\n\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: %s <so.pufu>\n", argv[0]);
        return 1;
    }

    // Configurar manejo de señales
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Mostrar mensaje de inicio
    printf("\n=== Pufu Bootloader ===\n");
    printf("Iniciando sistema...\n");

    // Inicializar sistema de nodos
    PufuNodeSystem* system = pufu_node_system_init();
    if (!system) {
        printf("Error: No se pudo inicializar el sistema de nodos\n");
        return 1;
    }

    // Mostrar animación de carga
    show_loading_animation();

    // Cargar nodo árbitro (so.pufu)
    PufuNode* arbiter = pufu_node_load(system, argv[1]);
    if (!arbiter) {
        printf("Error: No se pudo cargar el nodo inicial: %s\n", argv[1]);
        pufu_node_system_cleanup(system);
        return 1;
    }

    // Establecer como árbitro
    if (pufu_node_set_arbiter(system, arbiter) < 0) {
        printf("Error: No se pudo establecer el nodo inicial como árbitro\n");
        pufu_node_system_cleanup(system);
        return 1;
    }

    printf("Nodo inicial cargado: %s\n", arbiter->filename);
    printf("Cediendo control al nodo inicial...\n\n");

    // Ejecutar el nodo árbitro (so.pufu)
    if (pufu_node_execute(system->arbiter) < 0) {
        printf("Error: No se pudo ejecutar el nodo inicial\n");
        pufu_node_system_cleanup(system);
        return 1;
    }

    // Bucle principal
    while (running) {
        // Verificar cambios en todos los nodos
        int changes = pufu_node_system_check(system);
        if (changes > 0) {
            pufu_node_execute(system->arbiter);
        }
        usleep(100000);  // 100ms
    }

    printf("\nCerrando sistema...\n");
    pufu_node_system_cleanup(system);
    return 0;
} 
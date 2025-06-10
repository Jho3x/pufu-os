#include "pufu/hot_reload.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Uso: %s <archivo.pufu>\n", argv[0]);
        return 1;
    }

    // Inicializar el sistema de hot-reload
    PufuHotReload* reload = pufu_hot_reload_init(argv[1]);
    if (!reload) {
        printf("Error al inicializar hot-reload\n");
        return 1;
    }

    // Iniciar el monitoreo
    if (pufu_hot_reload_start(reload) < 0) {
        printf("Error al iniciar hot-reload\n");
        pufu_hot_reload_cleanup(reload);
        return 1;
    }

    printf("Monitoreando %s...\n", argv[1]);
    printf("Modifica el archivo para ver los cambios en tiempo real\n");
    printf("Presiona Ctrl+C para salir\n");

    // Bucle principal
    while (1) {
        // Verificar cambios
        int result = pufu_hot_reload_check(reload);
        if (result < 0) {
            printf("Error al verificar cambios\n");
            break;
        } else if (result > 0) {
            printf("\nArchivo modificado! Nuevas instrucciones:\n");
            // Aquí podrías imprimir las instrucciones o ejecutarlas
            for (int i = 0; i < reload->parser->count; i++) {
                PufuInstruction* inst = &reload->parser->instructions[i];
                printf("%s %s", inst->op, inst->reg1);
                if (inst->reg2[0]) printf(" %s", inst->reg2);
                if (inst->value[0]) printf(" %s", inst->value);
                printf("\n");
            }
        }

        // Pequeña pausa para no saturar la CPU
        usleep(100000);  // 100ms
    }

    // Limpiar recursos
    pufu_hot_reload_cleanup(reload);
    return 0;
} 
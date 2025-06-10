#ifndef PUFU_SOCKET_H
#define PUFU_SOCKET_H

#include <stdint.h>

// Estructura para la interfaz común de sockets
typedef struct {
    // Inicializar el socket
    int (*init)(void);
    
    // Ejecutar código Pufu
    int (*execute)(const char* code);
    
    // Cargar un archivo Pufu
    int (*load_file)(const char* filename);
    
    // Obtener el nombre de la arquitectura
    const char* (*get_arch_name)(void);
    
    // Obtener el tamaño de palabra
    int (*get_word_size)(void);
    
    // Limpiar recursos del socket
    void (*cleanup)(void);
} PufuSocket;

// Función para obtener el socket actual
PufuSocket* pufu_get_current_socket(void);

// Función para establecer el socket actual
void pufu_set_current_socket(PufuSocket* socket);

// Función para obtener el socket ARM
PufuSocket* pufu_get_arm_socket(void);

#endif // PUFU_SOCKET_H
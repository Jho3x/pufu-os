#ifndef PUFU_DYN_LOADER_H
#define PUFU_DYN_LOADER_H

#include "pufu/socket.h"

// Inicializa el sistema de carga dinámica
int pufu_dyn_loader_init(void);

// Carga un socket desde un archivo .so
// Retorna 0 si éxito, -1 si error
int pufu_load_socket(const char *path);

// Recarga el socket (Hot Swap)
// Retorna 0 si éxito, -1 si error (mantiene el socket anterior)
int pufu_reload_socket(const char *path);

// Obtiene el socket activo cargado dinámicamente
PufuSocket *pufu_loader_get_current(void);
// Alias para compatibilidad std
PufuSocket *pufu_get_current_socket(void);

// Limpia recursos del loader
void pufu_dyn_loader_cleanup(void);

#endif // PUFU_DYN_LOADER_H

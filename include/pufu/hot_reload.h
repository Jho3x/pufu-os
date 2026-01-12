#ifndef PUFU_HOT_RELOAD_H
#define PUFU_HOT_RELOAD_H

#include "pufu/parser.h"

// Estructura para el sistema de hot-reload
typedef struct {
  PufuParser *parser;   // Parser actual
  const char *filename; // Archivo a monitorear
  int watch_fd;         // Descriptor de inotify
  int is_running;       // Flag de ejecución
} PufuHotReload;

// Inicializar el sistema de hot-reload
PufuHotReload *pufu_hot_reload_init(const char *filename);

// Iniciar el monitoreo del archivo
int pufu_hot_reload_start(PufuHotReload *reload);

// Detener el monitoreo
void pufu_hot_reload_stop(PufuHotReload *reload);

// Verificar si hubo cambios
int pufu_hot_reload_check(PufuHotReload *reload);

// Liberar recursos
void pufu_hot_reload_cleanup(PufuHotReload *reload);

#endif // PUFU_HOT_RELOAD_H
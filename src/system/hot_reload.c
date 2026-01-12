#include "pufu/hot_reload.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

// Inicializar el sistema de hot-reload
PufuHotReload *pufu_hot_reload_init(const char *filename) {
  PufuHotReload *reload = malloc(sizeof(PufuHotReload));
  if (!reload)
    return NULL;

  // Inicializar el parser
  reload->parser = pufu_parser_init();
  if (!reload->parser) {
    free(reload);
    return NULL;
  }

  // Guardar el nombre del archivo
  reload->filename = strdup(filename);
  if (!reload->filename) {
    pufu_parser_cleanup(reload->parser);
    free(reload);
    return NULL;
  }

  // Inicializar inotify (non-blocking)
  reload->watch_fd = inotify_init1(IN_NONBLOCK);
  if (reload->watch_fd < 0) {
    perror("Debug: inotify_init1 failed");
    free((void *)reload->filename);
    pufu_parser_cleanup(reload->parser);
    free(reload);
    return NULL;
  }

  reload->is_running = 0;
  return reload;
}

// Iniciar el monitoreo del archivo
int pufu_hot_reload_start(PufuHotReload *reload) {
  if (!reload || reload->is_running)
    return -1;

  // Agregar el archivo al monitoreo
  int wd = inotify_add_watch(reload->watch_fd, reload->filename,
                             IN_MODIFY | IN_CLOSE_WRITE);
  if (wd < 0)
    return -1;

  reload->is_running = 1;

  // Parsear el archivo inicial
  if (pufu_parse_file(reload->parser, reload->filename) < 0) {
    inotify_rm_watch(reload->watch_fd, wd);
    reload->is_running = 0;
    return -1;
  }

  return 0;
}

// Función para verificar cambios en el archivo
int pufu_hot_reload_check(PufuHotReload *reload) {
  if (!reload || !reload->is_running)
    return -1;

  char buffer[EVENT_BUF_LEN];
  int length = read(reload->watch_fd, buffer, EVENT_BUF_LEN);

  if (length < 0) {
    if (errno == EAGAIN)
      return 0; // No hay eventos
    return -1;  // Error real
  }

  int i = 0;
  while (i < length) {
    struct inotify_event *event = (struct inotify_event *)&buffer[i];

    if (event->mask & (IN_MODIFY | IN_CLOSE_WRITE)) {
      // Limpiar el parser actual
      pufu_parser_cleanup(reload->parser);
      reload->parser = pufu_parser_init();

      // Re-parsear el archivo
      if (pufu_parse_file(reload->parser, reload->filename) < 0) {
        return -1;
      }

      return 1; // Archivo modificado y re-parseado
    }

    i += EVENT_SIZE + event->len;
  }

  return 0; // No hubo cambios
}

// Detener el monitoreo
void pufu_hot_reload_stop(PufuHotReload *reload) {
  if (!reload || !reload->is_running)
    return;

  inotify_rm_watch(reload->watch_fd, IN_ALL_EVENTS);
  reload->is_running = 0;
}

// Liberar recursos
void pufu_hot_reload_cleanup(PufuHotReload *reload) {
  if (!reload)
    return;

  if (reload->is_running) {
    pufu_hot_reload_stop(reload);
  }

  close(reload->watch_fd);
  free((void *)reload->filename);
  pufu_parser_cleanup(reload->parser);
  free(reload);
}
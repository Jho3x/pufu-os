#include "pufu/dyn_loader.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estado del loader
static void *current_handle = NULL;
static PufuSocket *current_socket = NULL;

// Backup para rollback (Futura implementación Watchdog)
// static void *backup_handle = NULL;
// static PufuSocket *backup_socket = NULL;

int pufu_dyn_loader_init(void) {
  // Nada especial por ahora
  return 0;
}

int pufu_load_socket(const char *path) {
  printf("[Loader] Cargando socket dinámico: %s\n", path);

  // Cargar la librería
  void *handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
  if (!handle) {
    fprintf(stderr, "[Loader] Error dlopen: %s\n", dlerror());
    return -1;
  }

  // Buscar el símbolo de entrada
  // Asumimos que todos los sockets exportan 'pufu_get_arm_socket'
  // TODO: Hacer este nombre genérico 'pufu_get_socket_interface'
  PufuSocket *(*get_socket_fn)(void) = dlsym(handle, "pufu_get_arm_socket");

  if (!get_socket_fn) {
    fprintf(stderr, "[Loader] Error dlsym: %s\n", dlerror());
    dlclose(handle);
    return -1;
  }

  // Obtener la interfaz
  PufuSocket *new_socket = get_socket_fn();
  if (!new_socket) {
    fprintf(stderr, "[Loader] Error: El socket retornó NULL\n");
    dlclose(handle);
    return -1;
  }

  // Inicializar el nuevo socket
  if (new_socket->init() < 0) {
    fprintf(stderr, "[Loader] Error: Falló init() del nuevo socket\n");
    dlclose(handle);
    return -1;
  }

  // Si había uno anterior, limpiarlo
  if (current_socket) {
    printf("[Loader] Cerrando socket anterior...\n");
    current_socket->cleanup();
    dlclose(current_handle);
  }

  // Switch
  current_handle = handle;
  current_socket = new_socket;

  printf("[Loader] Socket cargado exitosamente.\n");
  return 0;
}

#include "pufu/watchdog.h"

// ...

static void rollback_swap(void) {
  printf("[Loader] EXECUTION ROLLBACK! Reverting to previous socket...\n");
  if (current_socket) {
    // En un caso real, aquí restauraríamos el puntero 'backup_socket'
    // Por ahora, como el crash puede ser fatal, este rollback quizás solo
    // intente reiniciar el driver o abortar limpiamente.
    // Pero si tenemos backup_handle, podemos restaurarlo.
    // TODO: Implementar logica completa de backup A/B
  }
}

int pufu_reload_socket(const char *path) {
  printf("[Loader] Iniciando Hot Swap con: %s\n", path);

  // Armar Watchdog (3000ms timeout para cargar e init)
  pufu_watchdog_set_rollback(rollback_swap);
  pufu_watchdog_arm(3000);

  void *new_handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
  if (!new_handle) {
    fprintf(stderr,
            "[Loader] Falló carga del candidato. Abortando swap. Razón: %s\n",
            dlerror());
    pufu_watchdog_disarm();
    return -1;
  }

  PufuSocket *(*get_socket_fn)(void) = dlsym(new_handle, "pufu_get_arm_socket");
  if (!get_socket_fn) {
    dlclose(new_handle);
    pufu_watchdog_disarm();
    return -1;
  }

  PufuSocket *new_socket =
      get_socket_fn(); // renamed from 'new' (c++ keyword safety)

  // 1. Version Check
  if (new_socket->api_version != PUFU_SOCKET_API_VERSION) {
    fprintf(stderr,
            "[Loader] Version Mismatch! Kernel: %d, Driver: %d. Aborting.\n",
            PUFU_SOCKET_API_VERSION, new_socket->api_version);
    dlclose(new_handle);
    pufu_watchdog_disarm();
    return -1;
  }

  // 2. State Preservation (Save from Old)
  void *state_buffer = NULL;
  size_t state_size = 0;

  if (current_socket && current_socket->get_state_size &&
      current_socket->save_state) {
    state_size = current_socket->get_state_size();
    if (state_size > 0) {
      state_buffer = malloc(state_size);
      if (state_buffer) {
        if (current_socket->save_state(state_buffer, state_size) < 0) {
          fprintf(stderr, "[Loader] Warning: Failed to save state.\n");
          free(state_buffer);
          state_buffer = NULL;
        }
      }
    }
  }

  // 3. Init New
  if (new_socket->init() < 0) {
    if (state_buffer)
      free(state_buffer);
    dlclose(new_handle);
    pufu_watchdog_disarm();
    return -1;
  }

  // 4. State Restoration (Restore to New)
  if (state_buffer && new_socket->restore_state) {
    if (new_socket->restore_state(state_buffer, state_size) < 0) {
      fprintf(
          stderr,
          "[Loader] Warning: Failed to restore state (Version/Size mismatch "
          "likely).\n");
    } else {
      printf("[Loader] State migration successful (%zu bytes).\n", state_size);
    }
    free(state_buffer);
  }

  // 5. Swap
  if (current_socket) {
    current_socket->cleanup();
    dlclose(current_handle);
  }

  current_handle = new_handle;
  current_socket = new_socket;

  // Éxito, desarmar perro guardián
  pufu_watchdog_disarm();

  printf("[Loader] Hot Swap Completado.\n");
  return 0;
}

PufuSocket *pufu_loader_get_current(void) { return current_socket; }

// Alias para compatibilidad con el resto del sistema
PufuSocket *pufu_get_current_socket(void) { return current_socket; }

void pufu_dyn_loader_cleanup(void) {
  if (current_socket) {
    current_socket->cleanup();
    current_socket = NULL;
  }
  if (current_handle) {
    dlclose(current_handle);
    current_handle = NULL;
  }
}

int pufu_dyn_loader_download(const char *url, const char *dest) {
  printf("[Loader] Downloading Update...\n");
  printf("  URL: %s\n", url);
  printf("  Dest: %s\n", dest);
  
  // Security Warning: system() is dangerous in production.
  // We use it here for simulation purposes.
  char cmd[512];
  // Using curl with -s (silent) but -L (follow redirects)
  snprintf(cmd, sizeof(cmd), "curl -s -L -o \"%s\" \"%s\"", dest, url);
  
  int ret = system(cmd);
  if (ret != 0) {
    fprintf(stderr, "[Loader] Download Failed (Exit Code: %d)\n", ret);
    return -1;
  }
  
  printf("[Loader] Download Successful.\n");
  return 0;
}

#include "pufu/dyn_loader.h"
#include "pufu/loader.h"
#include "pufu/logger.h"
#include "pufu/node.h"
#include "pufu/terminal.h"
#include "pufu/trinity.h"
#include "pufu/watchdog.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PID_FILE "pufu.pid"
static int running = 1;

// Forward decl
void pufu_node_system_cleanup(PufuNodeSystem *system);

// Global Shutdown Function (Called by Signal or Syscall)
void pufu_os_shutdown(void) {
  // Unsafe printf removed to prevent deadlock in signal handler
  // printf("\n=== Shutting down Pufu OS ===\n");

  // Attempt to dump log before cleanup
  pufu_logger_dump("system.log");

  // Remove PID file
  unlink(PID_FILE);

  // Restore Terminal
  pufu_terminal_restore();

  // Trinity Cleanup (if active)
  trinity_shutdown();

  printf("[Kernel] System Halted.\n");
  exit(0);
}

void handle_signal(int sig) {
  (void)sig;
  // SAFE SHUTDOWN: set flag, let main loop exit.
  // printf is unsafe here if main loop is printing.
  running = 0;
}

void handle_crash(int sig) {
  printf("\n[Kernel] FATAL ERROR: Signal %d caught!\n", sig);
  printf("[Kernel] Dumping Crash Log to 'crash.log'...\n");
  pufu_logger_append("FATAL: System Crash Triggered.");
  pufu_logger_dump("crash.log");

  // Attempt clean shutdown? Or partial?
  pufu_terminal_restore();
  exit(1);
}

void check_pid_file() {
  FILE *f = fopen(PID_FILE, "r");
  if (f) {
    int pid;
    if (fscanf(f, "%d", &pid) == 1) {
      // Check if process is actually running
      if (kill(pid, 0) == 0) {
        fprintf(stderr, "Fatal: Pufu OS is already running (PID %d).\n", pid);
        fclose(f);
        exit(1);
      }
    }
    fclose(f);
  }
  // Create PID file
  f = fopen(PID_FILE, "w");
  if (f) {
    fprintf(f, "%d", getpid());
    fclose(f);
  }
}

// Función para mostrar una animación de carga
void show_loading_animation(void) {
  const char *frames[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
  const int num_frames = sizeof(frames) / sizeof(frames[0]);

  printf("\n");
  for (int i = 0; i < 20; i++) {
    printf("\r%s Cargando nodo inicial...", frames[i % num_frames]);
    fflush(stdout);
    usleep(100000); // 100ms
  }
  printf("\n\n");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Uso: %s <bootloader.pufu>\n", argv[0]);
    return 1;
  }

  // Configurar manejo de señales
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
  signal(SIGSEGV, handle_crash);
  signal(SIGABRT, handle_crash);

  pufu_logger_init();

  // Disable stdout buffering for responsive terminal
  // Disable stdout buffering for responsive terminal in Colab
  setvbuf(stdout, NULL, _IONBF, 0);

  // Mostrar mensaje de inicio
  pufu_log("\n=== Pufu Bootloader ===");
  pufu_log("Iniciando carga dinámica del socket...");

  // Inicializar Watchdog
  pufu_watchdog_init();

  // Inicializar Loader
  pufu_dyn_loader_init();
  const char *default_socket = "bin/drivers/socket_arm.so";
  if (pufu_load_socket(default_socket) < 0) {
    // Intentar path relativo si estamos en bin
    if (pufu_load_socket("drivers/socket_arm.so") < 0) {
      printf("FATAL: No se pudo cargar el socket %s\n", default_socket);
      return 1;
    }
  }

  // Prevent multiple instances
  check_pid_file();

  // Inicializar sistema de nodos
  PufuNodeSystem *system = pufu_node_system_init();
  if (!system) {
    pufu_log("Error: No se pudo inicializar el sistema de nodos");
    return 1;
  }

  // pufu_logger_init(); // Moved to top

  // Configurar terminal en modo RAW (Global)
  pufu_terminal_init();
  atexit(pufu_terminal_restore); // Failsafe cleanup

  // Mostrar animación de carga
  show_loading_animation();

  // Cargar nodo árbitro (so.pufu)
  PufuNode *arbiter = pufu_node_load(system, argv[1]);
  if (!arbiter) {
    printf("Error: No se pudo cargar el nodo inicial: %s\n", argv[1]);
    pufu_terminal_restore();
    pufu_node_system_cleanup(system);
    return 1;
  }

  // Establecer como árbitro
  if (pufu_node_set_arbiter(system, arbiter) < 0) {
    printf("Error: No se pudo establecer el nodo inicial como árbitro\n");
    pufu_terminal_restore();
    pufu_node_system_cleanup(system);
    return 1;
  }

  pufu_log("Nodo inicial cargado: %s", arbiter->filename);
  pufu_log("Cediendo control al nodo inicial...\n");

  // Ejecutar el nodo árbitro (so.pufu)
  if (pufu_node_execute(system, system->arbiter) < 0) {
    printf("Error: No se pudo ejecutar el nodo inicial\n");
    pufu_terminal_restore();
    pufu_node_system_cleanup(system);
    return 1;
  }

  // Bucle principal

  while (running) {
    // Verificar cambios en todos los nodos (hot-reload)
    int changes = pufu_node_system_check(system);

    if (changes > 0) {
      printf("Hot-reload detectado.\n");
    }

    int active_nodes = 0;
    PufuNode *current = system->nodes;
    while (current) {
      if (current->active) {
        active_nodes++;
        pufu_node_execute(system, current);
      }
      current = current->next;
    }
    fflush(stdout);

    // Si no hay nodos activos, el sistema murio
    if (active_nodes == 0) {
      if (running) { // Only print if not manually stopped
        printf("\nCerrando sistema (No active nodes)...\n");
      }
      break;
    }

    // Yield CPU briefly to allow OS to deliver signals and reduce CPU usage
    if (active_nodes > 0) {
      // En un sistema real esto seria un wait for interrupt o select/epoll
      // Aqui usamos un sleep moderado (10ms) para bajar el uso de CPU.
      usleep(10000);
    }
  }

  // Cleanup on exit
  printf("\n=== Shutting down Pufu ===\n");
  printf("[Bootloader] Cleaning up Node System...\n");
  pufu_logger_dump("system.log");
  pufu_node_system_cleanup(system);

  // Ensure Window is closed
  trinity_shutdown();

  printf("[Bootloader] Restoring Terminal...\n");
  pufu_terminal_restore();
  printf("[Bootloader] Bye!\n");
  return 0;
}
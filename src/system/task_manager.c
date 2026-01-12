#include "pufu/task_manager.h"
#include <stdio.h>
#include <stdlib.h>

PufuTaskManager *pufu_task_manager_init(PufuNodeSystem *system) {
  if (!system)
    return NULL;

  PufuTaskManager *tm = malloc(sizeof(PufuTaskManager));
  if (!tm)
    return NULL;

  tm->system = system;
  tm->active = 1;

  printf("TaskManager (Arbiter) Initialized.\n");
  return tm;
}

void pufu_task_manager_update(PufuTaskManager *tm) {
  if (!tm || !tm->active)
    return;

  // Lógica simple de Scheduler:
  // 1. Verificar si hay cambios en los nodos (Hot Reload)
  int changes = pufu_node_system_check(tm->system);

  // 2. Si hubo cambios, el árbitro decide qué hacer (por ahora, re-ejecutar el
  // árbitro)
  if (changes > 0) {
    printf("TaskManager: Detected changes, re-executing arbiter node.\n");
    if (tm->system->arbiter) {
      pufu_node_execute(tm->system, tm->system->arbiter);
    }
  }

  // Aquí se podría implementar la lógica de "Pipe Parallel" vs "Pipe Serial"
  // basada en la carga del sistema.
}

void pufu_task_manager_cleanup(PufuTaskManager *tm) {
  if (tm) {
    free(tm);
  }
}

#ifndef PUFU_TASK_MANAGER_H
#define PUFU_TASK_MANAGER_H

#include "pufu/node.h"

// Estructura del Árbitro (TaskManager)
typedef struct {
  PufuNodeSystem *system;
  int active;
} PufuTaskManager;

// Inicializar TaskManager
PufuTaskManager *pufu_task_manager_init(PufuNodeSystem *system);

// Ciclo principal de TaskManager (Scheduler)
void pufu_task_manager_update(PufuTaskManager *tm);

// Liberar TaskManager
void pufu_task_manager_cleanup(PufuTaskManager *tm);

#endif // PUFU_TASK_MANAGER_H

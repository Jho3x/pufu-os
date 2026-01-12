#ifndef PUFU_WATCHDOG_H
#define PUFU_WATCHDOG_H

// Inicializa el sistema de Watchdog
void pufu_watchdog_init(void);

// Arma el watchdog con un tiempo límite en milisegundos.
// Si no se llama a disarm antes de que expire, el sistema intentará un rollback
// o reinicio.
void pufu_watchdog_arm(int timeout_ms);

// Desarma el watchdog (éxito de la operación crítica).
void pufu_watchdog_disarm(void);

// Registra la función de rollback a ejecutar si el watchdog expira.
void pufu_watchdog_set_rollback(void (*rollback_func)(void));

#endif // PUFU_WATCHDOG_H

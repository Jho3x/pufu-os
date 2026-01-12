#ifndef PUFU_VIRTUAL_BUS_H
#define PUFU_VIRTUAL_BUS_H

#include <stdint.h>

// Estructura de una Señal (Signal)
typedef struct {
  uint16_t id;   // ID del tipo de señal (ej. 0x1100 para Red)
  uint16_t size; // Tamaño de los datos en nibbles
  uint8_t *data; // Datos crudos (nibble train)
} PufuSignal;

// Estructura de VirtualBus (Bus de Mensajes)
typedef struct {
  // Cola de señales (simplificada por ahora)
  PufuSignal *signal_queue;
  int queue_size;
} PufuVirtualBus;

// Inicializar VirtualBus
PufuVirtualBus *pufu_virtual_bus_init(void);

// Enviar una señal al bus
int pufu_virtual_bus_send(PufuVirtualBus *bus, uint16_t id, uint8_t *data,
                          uint16_t size);

// Procesar señales pendientes
void pufu_virtual_bus_process(PufuVirtualBus *bus);

// Liberar VirtualBus
void pufu_virtual_bus_cleanup(PufuVirtualBus *bus);

#endif // PUFU_VIRTUAL_BUS_H

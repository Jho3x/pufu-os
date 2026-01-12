#include "pufu/virtual_bus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PufuVirtualBus *pufu_virtual_bus_init(void) {
  PufuVirtualBus *bus = malloc(sizeof(PufuVirtualBus));
  if (!bus)
    return NULL;

  bus->signal_queue = NULL; // Por ahora sin cola real
  bus->queue_size = 0;

  printf("VirtualBus (IPC) Initialized.\n");
  return bus;
}

int pufu_virtual_bus_send(PufuVirtualBus *bus, uint16_t id, uint8_t *data,
                          uint16_t size) {
  (void)data; // Unused for now
  if (!bus)
    return -1;

  // En una implementación real, esto encolaría la señal.
  // Por ahora, la procesamos inmediatamente (modo síncrono).
  printf("VirtualBus: Signal Received [ID: 0x%04X, Size: %d]\n", id, size);

  // Aquí iría la lógica de enrutamiento (Routing Table)
  // Ej: Si ID == 0x1100 -> Enviar a Node_Net

  return 0;
}

void pufu_virtual_bus_process(PufuVirtualBus *bus) {
  // Procesar cola de señales
  (void)bus;
}

void pufu_virtual_bus_cleanup(PufuVirtualBus *bus) {
  if (bus) {
    free(bus);
  }
}

#ifndef PUFU_CRYSTAL_H
#define PUFU_CRYSTAL_H

#include <stdint.h>

// Estructura para el motor Crystal
// Estructura para una compuerta en el Netlist
typedef struct {
  int id;              // ID único de la compuerta
  uint8_t opcode;      // Opcode (Válvula 1)
  uint8_t type;        // Tipo de compuerta (Válvula 2)
  int input1_id;       // ID de la entrada 1 (-1 si es constante/input externo)
  int input2_id;       // ID de la entrada 2
  int input1_val;      // Valor constante si input1_id es -1
  int input2_val;      // Valor constante si input2_id es -1
  uint8_t output_val;  // Valor de salida actual (cache)
  char *helicoid_data; // Datos del Helicoide (String/ADN)
} PufuGate;

// Estructura para el Netlist (Grafo de compuertas)
typedef struct {
  PufuGate *gates;
  int gate_count;
  int gate_capacity;
  int output_gate_id; // ID de la compuerta que representa la salida final
} PufuNetlist;

typedef struct {
  int active;
  PufuNetlist *current_netlist;
} PufuCrystal;

PufuCrystal *pufu_crystal_init(void);
void pufu_crystal_cleanup(PufuCrystal *crystal);

// Ejecutar una compuerta individual (Low level)
uint8_t pufu_crystal_gate_execute(uint8_t op, uint8_t a, uint8_t b);

// Parsear un archivo .crystal y cargar el Netlist
int pufu_crystal_load_netlist(PufuCrystal *crystal, const char *filename);

// Ejecutar el ciclo del Netlist
uint8_t pufu_crystal_step(PufuCrystal *crystal);

// Liberar Crystal
void pufu_crystal_cleanup(PufuCrystal *crystal);

#endif // PUFU_CRYSTAL_H

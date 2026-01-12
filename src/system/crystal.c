#include "pufu/crystal.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper para convertir nombre de compuerta a tipo ID
static uint8_t get_gate_type(const char *name) {
  if (strcmp(name, "ZER") == 0)
    return 0x0;
  if (strcmp(name, "AND") == 0)
    return 0x1;
  if (strcmp(name, "BIE") == 0)
    return 0x2;
  if (strcmp(name, "TIE") == 0)
    return 0x3;
  if (strcmp(name, "QUA") == 0)
    return 0x4;
  if (strcmp(name, "STA") == 0)
    return 0x5;
  if (strcmp(name, "XOR") == 0)
    return 0x6;
  if (strcmp(name, "HEP") == 0)
    return 0x7;
  if (strcmp(name, "OCT") == 0)
    return 0x8;
  if (strcmp(name, "JOE") == 0)
    return 0x9;
  if (strcmp(name, "RAI") == 0)
    return 0xA;
  if (strcmp(name, "NOD") == 0)
    return 0xB;
  if (strcmp(name, "DOZ") == 0)
    return 0xC;
  if (strcmp(name, "AXE") == 0)
    return 0xD;
  if (strcmp(name, "NAD") == 0)
    return 0xE;
  if (strcmp(name, "ONE") == 0)
    return 0xF;
  return 0x0;
}

PufuCrystal *pufu_crystal_init(void) {
  PufuCrystal *crystal = malloc(sizeof(PufuCrystal));
  if (!crystal)
    return NULL;

  crystal->active = 1;
  crystal->current_netlist = NULL;
  printf("Crystal (Soft-FPGA Engine) Initialized.\n");
  return crystal;
}

// Ejecutar una compuerta lógica (Soft-FPGA)
uint8_t pufu_crystal_gate_execute(uint8_t op, uint8_t a, uint8_t b) {
  // Normalizar entradas a 1 bit
  a = a ? 1 : 0;
  b = b ? 1 : 0;

  switch (op & 0x0F) { // Asegurar que solo usamos 4 bits
  case 0x0:
    return 0; // ZER (Zero)
  case 0x1:
    return a & b; // AND
  case 0x2:
    return a & !b; // BIE
  case 0x3:
    return a; // TIE
  case 0x4:
    return (!a) & b; // QUA
  case 0x5:
    return b; // STA
  case 0x6:
    return a ^ b; // XOR
  case 0x7:
    return a | b; // HEP
  case 0x8:
    return !(a | b); // OCT
  case 0x9:
    return !(a ^ b); // JOE
  case 0xA:
    return !b; // RAI
  case 0xB:
    return a | (!b); // NOD
  case 0xC:
    return !a; // DOZ
  case 0xD:
    return (!a) | b; // AXE
  case 0xE:
    return !(a & b); // NAD
  case 0xF:
    return 1; // ONE
  default:
    return 0;
  }
}

// Buscar compuerta por ID
static PufuGate *find_gate(PufuNetlist *netlist, int id) {
  for (int i = 0; i < netlist->gate_count; i++) {
    if (netlist->gates[i].id == id)
      return &netlist->gates[i];
  }
  return NULL;
}

// DJB2 Hash for string IDs
static int hash_string(const char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return (int)(hash & 0x7FFFFFFF);   // Keep it positive int
}

// Parsear input (puede ser ID "g1" o valor "0"/"1")
static void parse_input(const char *token, int *id_out, int *val_out) {
  // If it starts with a letter, it's an ID
  if (isalpha(token[0])) {
    *id_out = hash_string(token);
    *val_out = 0;
  } else {
    // Otherwise it's a numeric value (0 or 1)
    *id_out = -1;
    *val_out = atoi(token);
  }
}

int pufu_crystal_load_netlist(PufuCrystal *crystal, const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file)
    return -1;

  if (crystal->current_netlist) {
    free(crystal->current_netlist->gates);
    free(crystal->current_netlist);
  }

  PufuNetlist *netlist = malloc(sizeof(PufuNetlist));
  netlist->gate_capacity = 64;
  netlist->gates = malloc(sizeof(PufuGate) * netlist->gate_capacity);
  netlist->gate_count = 0;
  netlist->output_gate_id = -1;

  char line[256];
  int inside_claw = 0;
  // Check if we need to look for a block (heuristic: if filename ends in .pufu)
  int is_pufu_file = (strstr(filename, ".pufu") != NULL);

  while (fgets(line, sizeof(line), file)) {
    // Skip comments and empty lines
    if (line[0] == '#' || line[0] == '\n')
      continue;

    // Handle blocks
    if (strstr(line, "_claw::init")) {
      inside_claw = 1;
      continue;
    }
    if (strstr(line, "_claw::end")) {
      inside_claw = 0;
      break;
    }

    // If it's a .pufu file and we are not inside a block, skip
    if (is_pufu_file && !inside_claw)
      continue;

    char t1[32] = "0", t2[32] = "0", t3[32] = "0", t4[64] = "0", t5[64] = "0";

    // Manual parsing to handle quoted strings with spaces
    char *ptr = line;
    while (*ptr && isspace(*ptr))
      ptr++;
    sscanf(ptr, "%31s", t1);
    ptr += strlen(t1);
    while (*ptr && isspace(*ptr))
      ptr++;
    sscanf(ptr, "%31s", t2);
    ptr += strlen(t2);
    while (*ptr && isspace(*ptr))
      ptr++;
    sscanf(ptr, "%31s", t3);
    ptr += strlen(t3);
    while (*ptr && isspace(*ptr))
      ptr++;

    // Parse t4 (IN1) - check for quotes
    if (*ptr == '"') {
      char *start = ptr;
      ptr++; // Skip quote
      while (*ptr && *ptr != '"')
        ptr++;
      if (*ptr == '"')
        ptr++; // Skip closing quote
      int len = ptr - start;
      if (len >= 63)
        len = 63;
      strncpy(t4, start, len);
      t4[len] = 0;
    } else {
      sscanf(ptr, "%63s", t4);
      ptr += strlen(t4);
    }

    while (*ptr && isspace(*ptr))
      ptr++;
    sscanf(ptr, "%63s", t5);

    if (strcmp(t1, "OUTPUT") == 0) {
      int id, val;
      // OUTPUT AXE TIE final 0 -> t2=AXE, t3=TIE, t4=final
      // We just care about the ID to track
      parse_input(t4, &id, &val);
      netlist->output_gate_id = id;
      continue;
    }

    if (isalpha(t1[0])) {
      // Definición de compuerta: id OPCODE TYPE in1 in2
      int id;
      int dummy;
      parse_input(t1, &id, &dummy);

      PufuGate *gate = &netlist->gates[netlist->gate_count++];

      gate->id = id;
      gate->opcode = get_gate_type(t2); // Parse Opcode
      gate->type = get_gate_type(t3);   // Parse Type

      // Check for string literal in t4 (IN1)
      if (t4[0] == '"') {
        // Remove quotes
        size_t len = strlen(t4);
        if (len > 2) {
          t4[len - 1] = '\0';                   // Remove trailing quote
          gate->helicoid_data = strdup(t4 + 1); // Remove leading quote
        } else {
          gate->helicoid_data = strdup("");
        }
        gate->input1_id = -1;
        gate->input1_val = 0; // Or maybe use data presence as value?
      } else {
        gate->helicoid_data = NULL;
        parse_input(t4, &gate->input1_id, &gate->input1_val);
      }

      parse_input(t5, &gate->input2_id, &gate->input2_val);
      gate->output_val = 0;
    }
  }

  fclose(file);
  crystal->current_netlist = netlist;
  printf("Crystal: Loaded netlist with %d gates.\n", netlist->gate_count);
  return 0;
}

uint8_t pufu_crystal_step(PufuCrystal *crystal) {
  if (!crystal || !crystal->current_netlist)
    return 0;

  PufuNetlist *net = crystal->current_netlist;
  int changed = 1;
  int max_iters = 100; // Evitar loops infinitos por ahora

  // Propagar señales hasta estabilidad
  while (changed && max_iters-- > 0) {
    changed = 0;
    for (int i = 0; i < net->gate_count; i++) {
      PufuGate *g = &net->gates[i];

      uint8_t val1 = g->input1_val;
      char *data1 = NULL;

      if (g->input1_id != -1) {
        PufuGate *src = find_gate(net, g->input1_id);
        if (src) {
          val1 = src->output_val;
          data1 = src->helicoid_data;
        }
      } else {
        // If input1 is constant, check if we have local helicoid data
        // Actually, if input1 was a string, we stored it in g->helicoid_data
        // already? No, we stored it in g->helicoid_data. So if g->helicoid_data
        // is set, that IS our data.
        if (g->helicoid_data)
          data1 = g->helicoid_data;
      }

      uint8_t val2 = g->input2_val;
      if (g->input2_id != -1) {
        PufuGate *src = find_gate(net, g->input2_id);
        if (src)
          val2 = src->output_val;
      }

      // Execute Logic (Valve 2)
      uint8_t new_out = pufu_crystal_gate_execute(g->type, val1, val2);

      // Handle Opcode (Valve 1)

      // DOZ (12) = IN
      // If we have helicoid data (static input), pass it through?
      // Or if we are DOZ, maybe we should prompt user if no data?
      // For this task: "La brana pide al usuario un nombre".
      // Let's assume if DOZ has no data, we might ask. But here we have
      // "nimbus" as static data in the pufu file. So DOZ just holds the data.

      // AXE (13) = OUT
      if (g->opcode == 0xD) { // AXE
                              // If we have data from input1, print it.
                              // Print only once per step or check change?
        // For simplicity, print if it's the first time or something.
        // But we are in a loop.
        // Let's print only in the post-stabilization loop to avoid spam.
      }

      // STA (5) = STORE
      if (g->opcode == 0x5) { // STA
        // Save data1 to file.
        // Where? Maybe hardcoded "nube.txt" for now.
        if (data1) {
          FILE *f = fopen("nube.txt", "w");
          if (f) {
            fprintf(f, "nombre : %s\n", data1);
            fclose(f);
            // printf("Crystal: Saved to nube.txt\n");
          }
        }
      }

      if (new_out != g->output_val) {
        g->output_val = new_out;
        changed = 1;
      }

      // Propagate data?
      // If this gate is TIE (Identity), should it pass helicoid_data to its
      // output? We need a way to store output data on the gate. Current struct
      // has `helicoid_data` which we used for static input. We can reuse it for
      // dynamic output data if we duplicate the string.
      if (g->type == 0x3) { // TIE
        if (data1 && !g->helicoid_data) {
          g->helicoid_data = strdup(data1); // Propagate data
        }
      }
    }
  }

  // Post-stabilization: Check AXE gates and print
  for (int i = 0; i < net->gate_count; i++) {
    PufuGate *g = &net->gates[i];
    if (g->opcode == 0xD) { // AXE
      if (g->helicoid_data) {
        printf("nombre : %s\n", g->helicoid_data);
      } else if (g->input1_id != -1) {
        PufuGate *src = find_gate(net, g->input1_id);
        if (src && src->helicoid_data) {
          printf("nombre : %s\n", src->helicoid_data);
        } else {
          printf("%d", g->output_val); // Fallback to bit
        }
      } else {
        printf("%d", g->output_val);
      }
    }
  }
  printf("\n"); // Newline after printing all bits

  if (net->output_gate_id != -1) {
    PufuGate *out = find_gate(net, net->output_gate_id);
    return out ? out->output_val : 0;
  }
  return 0;
}

void pufu_crystal_cleanup(PufuCrystal *crystal) {
  if (crystal) {
    if (crystal->current_netlist) {
      free(crystal->current_netlist->gates);
      free(crystal->current_netlist);
    }
    free(crystal);
  }
}

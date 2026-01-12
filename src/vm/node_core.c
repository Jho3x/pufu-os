#include "pufu/loader.h"
#include "pufu/node.h"
#include "pufu/terminal.h"
#include "pufu/virtual_bus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Helpers ---

long long get_time_ms() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int get_key(void) { return pufu_terminal_get_key(); }

// --- System Initialization ---

PufuNodeSystem *pufu_node_system_init(void) {
  PufuNodeSystem *system = malloc(sizeof(PufuNodeSystem));
  if (!system)
    return NULL;

  system->arbiter = NULL;
  system->nodes = NULL;
  system->bus = pufu_virtual_bus_init();

  pufu_log("pufu_so node parser initialized");

  return system;
}

// --- Node Loading & Management ---

PufuNode *pufu_node_load(PufuNodeSystem *system, const char *filename) {
  if (!system)
    return NULL;

  // Create Node
  PufuNode *node = pufu_create_node(filename, -1);
  if (!node)
    return NULL;

  // Load Content
  if (node->type == PUFU_NODE_ASSEMBLER ||
      node->type == PUFU_NODE_TRINITY_SCENE) {
    if (pufu_parse_file(node->parser, filename) < 0) {
      printf("Error parsing file: %s\n", filename);
      // Cleanup
      pufu_parser_cleanup(node->parser);
      free(node->filename);
      free(node);
      return NULL;
    }
  } else if (node->type == PUFU_NODE_CRYSTAL) {
    // Crystal loading logic handled in init or we need a loader function
    // For now assume crystal init did enough or we need pufu_crystal_load
    // In legacy, crystal might load itself.
  }

  // Add to list
  node->next = system->nodes;
  system->nodes = node;

  return node;
}

int pufu_node_set_arbiter(PufuNodeSystem *system, PufuNode *node) {
  if (!system || !node)
    return -1;

  // Verify node is in system? Not strictly necessary but safe.
  // Set arbiter
  system->arbiter = node;
  node->is_arbiter = 1;
  return 0;
}

// --- Node Factory ---

// Detectar tipo de nodo usando Magic Header
static PufuNodeType detect_node_type(const char *filename) {
  // 1. Extension Check
  const char *dot = strrchr(filename, '.');
  if (dot && strcmp(dot, ".crystal") == 0) {
    return PUFU_NODE_CRYSTAL;
  }

  // 2. Magic Header / Content Check
  FILE *f = fopen(filename, "r");
  if (f) {
    char line[256];
    int lines_checked = 0;
    while (fgets(line, sizeof(line), f) && lines_checked < 20) {
      if (strstr(line, "_pufu::meow") || strstr(line, "_claw::init") ||
          strstr(line, "_pufu::scene")) {
        fclose(f);
        return PUFU_NODE_TRINITY_SCENE;
      }
      lines_checked++;
    }
    fclose(f);
  }

  return PUFU_NODE_ASSEMBLER;
}

PufuNode *pufu_create_node(const char *filename, int type_override) {
  PufuNode *node = malloc(sizeof(PufuNode));
  if (!node)
    return NULL;

  node->filename = strdup(filename);
  if (!node->filename) {
    free(node);
    return NULL;
  }

  node->type = (type_override != -1) ? (PufuNodeType)type_override
                                     : detect_node_type(filename);
  node->parser = NULL;
  node->crystal = NULL;
  node->body = NULL;
  node->reload = NULL;

  // Configuration based on Type
  if (node->type == PUFU_NODE_ASSEMBLER) {
    node->parser = pufu_parser_init();
  } else if (node->type == PUFU_NODE_CRYSTAL) {
    node->crystal = pufu_crystal_init();
    node->parser =
        pufu_parser_init(); // Crystal needs parser for netlist loading
  } else if (node->type == PUFU_NODE_TRINITY_SCENE) {
    // For a Scene, we actually load the Trinity Engine's parser
    node->parser = pufu_parser_init();
  }

  if ((node->type == PUFU_NODE_ASSEMBLER ||
       node->type == PUFU_NODE_TRINITY_SCENE) &&
      !node->parser) {
    free(node->filename);
    free(node);
    return NULL;
  }
  if (node->type == PUFU_NODE_CRYSTAL && !node->crystal) {
    free(node->filename);
    free(node);
    return NULL;
  }

  // Hot Reload is always useful
  node->reload = pufu_hot_reload_init(filename);

  node->next = NULL;
  node->is_arbiter = 0;
  node->loop_counter = 0;
  node->ip = 0;
  node->active = 1;
  node->wake_time = 0;
  node->cmp_flag = 0;
  node->input_pos = 0;
  memset(node->registers, 0, sizeof(node->registers));
  memset(node->input_buffer, 0, sizeof(node->input_buffer));
  node->input_pos = 0;

  // Default TWS to active one (or 0)
  // extern int pufu_tws_get_active(void); // Forward decl or include terminal.h
  // Removed extern implicit because terminal.h is included
  node->tws_id = pufu_tws_get_active();

  // IPC Queue Init
  node->ipc_head = 0;
  node->ipc_tail = 0;
  node->ipc_count = 0;

  return node;
}

// --- System Cleanup ---

void pufu_node_system_cleanup(PufuNodeSystem *system) {
  if (!system)
    return;

  PufuNode *current = system->nodes;
  while (current) {
    PufuNode *next = current->next;
    pufu_hot_reload_cleanup(current->reload);
    pufu_parser_cleanup(current->parser);
    free(current->filename);
    free(current);
    current = next;
  }

  // Clean up Trinity Engine resources
  extern void trinity_shutdown(void);
  trinity_shutdown();

  pufu_virtual_bus_cleanup(system->bus);
  free(system);
}

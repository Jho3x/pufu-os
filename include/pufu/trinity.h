#ifndef TRINITY_H
#define TRINITY_H

#include "trinity_math.h"
#include <stdbool.h>
#include <stdint.h>

// --- Definición de Tipos ---

typedef int32_t NodeID;

typedef enum {
  NODE_ROOT = 0,
  NODE_WINDOW,
  NODE_SPACE,
  NODE_CAMERA,
  NODE_LIGHT,
  NODE_MESH, // Cubo, Esfera, Tetraedro
  NODE_SKYBOX,
  NODE_EMITTER, // Fuego
  NODE_TEXT_3D,
  NODE_GEAR,       // Controlador Lógico
  NODE_PROCEDURAL, // Geometría Generada por Función
  NODE_UI_BUTTON,  // Interfaz de usuario (2D Overlay)
  NODE_UI_IMAGE,   // Imagen 2D independiente (UI)
  NODE_UI_WINDOW   // Contenedor de Ventana (Clipping + Frame)
} NodeType;

// Definición de función generadora: f(t) -> mesh
typedef void (*ProceduralFunc)(float t, float *out_vertices, int *out_count);

// --- API Pública ---

// Inicializa el subsistema Trinity (pero no abre ventana hasta que se crea el
// nodo Window)
void trinity_init();

// Bucle principal bloqueante (Ejecuta Gear.run)
// Returns exit code: 0=Close, 1=BG
int trinity_run(NodeID gear_node);

// Single Frame Step (Non-blocking)
// Returns: 1=Continue, 0=Close, 2=BG
int trinity_step(void);
void trinity_prepare(void);

// Limpieza

// Limpieza
void trinity_shutdown();

// Boot Visuals
void trinity_render_boot_sequence(void);
bool trinity_session_manager(void);

// --- Gestión de Nodos ---

// Crea un nodo y retorna su ID
NodeID trinity_create_node(const char *name, NodeType type);

// Vincula nodos (Padre -> Hijo). Ejemplo: Space -> Camera
void trinity_link_nodes(NodeID parent, NodeID child);

// --- Propiedades ---

void trinity_set_vec3(NodeID id, const char *prop_name, float x, float y,
                      float z);
void trinity_set_vec4(NodeID id, const char *prop_name, float r, float g,
                      float b, float a);
void trinity_get_vec4(NodeID id, const char *prop_name, float *out_vec);
void trinity_set_string(NodeID id, const char *prop_name, const char *val);
// void trinity_set_float(NodeID id, const char* prop_name, float val);
NodeID trinity_get_node_at(int x, int y);

// Comando especial para "control" (Gear controla Space/Window)
void trinity_bind_control(NodeID gear, NodeID controlled_node);

// Comando para añadir items a lista (Space.add)
void trinity_node_add_list(NodeID space, NodeID *children, int count);

// Asigna función generadora
void trinity_set_procedural_generator(NodeID id, ProceduralFunc func);
void trinity_bind_event(NodeID id, int event_type, const char *cmd);
bool trinity_get_binding(NodeID id, int event_type, char *out_cmd);
/* Generic Lookup */
NodeID trinity_get_node_id(const char *name);

// --- Event Handling ---
typedef struct {
  int type; // 4=MouseDown, 5=MouseUp, 6=MouseMove
  int x;
  int y;
  NodeID target_id; // -1 if none
} TrinityEvent;

void trinity_queue_event(int type, int x, int y, NodeID target);
bool trinity_dequeue_event(TrinityEvent *out_event);

// --- Backend Renderer API ---
bool trinity_renderer_init(int width, int height, bool headless);
int trinity_renderer_poll_events();
void trinity_renderer_frame_start();
void trinity_renderer_frame_end();
void trinity_renderer_destroy();

#endif // TRINITY_H

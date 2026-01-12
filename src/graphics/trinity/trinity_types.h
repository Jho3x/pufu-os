#ifndef TRINITY_TYPES_H
#define TRINITY_TYPES_H

#include "pufu/trinity.h"
#include <stddef.h>

#define MAX_NODES 1024
#define MAX_NAME_LEN 64
#define MAX_CHILDREN 32

// Flags de configuración por nodo
typedef enum {
  NODE_FLAG_NONE = 0,
  NODE_FLAG_VISIBLE = 1 << 0,
  NODE_FLAG_PERSIST = 1 << 1,       // Se guarda al hibernar
  NODE_FLAG_HIGH_PRECISION = 1 << 2 // Usa double en lógica (si aplica)
} NodeFlags;

// Definición de como interpretar el 'data_ptr'
typedef enum {
  DATA_NONE = 0,
  DATA_MESH_BUFFER,     // Vértices estáticos
  DATA_MESH_PROCEDURAL, // Función generadora
  DATA_CAMERA,
  DATA_LIGHT,
  DATA_EMITTER,
  DATA_WINDOW,
  DATA_LOGIC_GATE, // Compuerta lógica (Mesecons/Redstone)
  DATA_UI_BUTTON,  // Botón 2D
  DATA_UI_IMAGE,   // Imagen 2D
  DATA_FRAME       // Fractal Frame (Window/Desktop)
} DataType;

// ... (existing structs)

typedef struct {
  vec3 color_on;
  vec3 color_off;
  bool is_pressed;
  // UI Properties
  char label[64];
  uint32_t text_texture_id; // 0 if none
  int text_w;
  int text_h;
  // Icon Properties
  uint32_t icon_texture_id;
  int icon_w;
  int icon_h;

  // Font Metrics for Centering
  int text_ascent;
  int text_descent;
  int text_baseline;
  bool text_dirty;
  // Position/Size are managed by Node transform (scale = size)?
  // Or payload specific? Let's use payload for 2D coords.
  float x, y;
  float width, height;
  char on_click_cmd[128]; // Command to execute on click
} PayloadButton;

// Nodo Universal
typedef struct Node {
  NodeID id;
  char name[MAX_NAME_LEN];
  NodeType type; // Categoría general (Space, Actor, Prop)
  NodeFlags flags;

  // Transform (El Núcleo Físico)
  vec3 position;
  vec3 rotation; // Euler por ahora, Quaterniones luego
  vec3 scale;

  // Jerarquía
  NodeID parent_id;
  NodeID children[MAX_CHILDREN];
  int children_count;

  // Polimorfismo de Datos (La Piel / Lógica)
  DataType data_type;
  void *data_ptr;   // Puntero a la estructura específica (Mesh, Camera, etc)
  size_t data_size; // Para serialización cruda

  // Driver / Scripting
  // void (*driver_tick)(struct Node* self); // Puntero a función de lógica

} Node;

// --- Estructuras de Datos Específicos (Payloads) ---

typedef struct {
  char resource_path[128];
  unsigned int vao, vbo; // GLES Handles
} PayloadMeshBuffer;

typedef struct {
  float fov;
  vec3 target;
} PayloadCamera;

typedef struct {
  vec3 color;
  float intensity;
} PayloadLight;

typedef struct {
  int width, height;
} PayloadWindow;

// typedef moved to trinity.h

typedef struct {
  ProceduralFunc generator;
  float frequency;
  float amplitude;
  float phase;
} PayloadProcedural;

typedef struct {
  float x, y;
  float width, height;
  uint32_t texture_id;
  char path[128];
} PayloadImage;

typedef struct {
  char title[64]; // kept for legacy name or debug
  float x, y;
  float width, height;
  bool minimized;

  // Fractal Composition
  NodeID header_id; // e.g. Taskbar/Titlebar
  NodeID body_id;   // e.g. Canvas/Content

  bool movable; // True = Window, False = Desktop/Fixed

  vec3 bg_color; // Optional Background Color
} PayloadFrame;

#endif // TRINITY_TYPES_H

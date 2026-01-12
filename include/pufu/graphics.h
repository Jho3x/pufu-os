#ifndef PUFU_GRAPHICS_H
#define PUFU_GRAPHICS_H

#include <stdint.h>

// Inicializa el sistema gráfico (X11 + EGL)
// Retorna 0 si éxito, -1 si error
int pufu_graphics_init(int width, int height, const char *title);

// Limpia la pantalla con un color (r, g, b: 0.0 - 1.0)
void pufu_graphics_clear(float r, float g, float b);

// Intercambia los buffers (Double Buffering)
void pufu_graphics_swap(void);

// Procesa eventos de ventana (ej. cerrar)
// Retorna 0 si se debe cerrar, 1 si continúa
int pufu_graphics_poll(void);

// Texture API
int pufu_graphics_load_texture(const char *path, int *w, int *h);
void pufu_graphics_draw_texture(int texture_id, float x, float y, float w,
                                float h);

// 3D Model API
int pufu_graphics_load_model(const char *path);
void pufu_graphics_draw_model(int model_id, int texture_id, float x, float y,
                              float z, float scale, float ry);

// Genera una primitiva (Cubo) en el slot ID especificado
void pufu_create_primitive(int id);
void pufu_graphics_cleanup(void);

// --- Framebuffer Abstraction (HAL) ---
typedef struct {
  int width;
  int height;
  int stride;   // Bytes per row (usually width * 4)
  int format;   // 0 = RGBA8888
  void *pixels; // Raw pixel data
} PufuFramebuffer;

// Get the system framebuffer (Virtual or Physical)
PufuFramebuffer *pufu_video_get_fb(void);

#endif // PUFU_GRAPHICS_H

#ifndef OPENGL_ES_BACKEND_H
#define OPENGL_ES_BACKEND_H

#include <stdbool.h>

// Inicializa EGL.
// Si headless=true, intenta crear un contexto sin ventana (Pbuffer o
// Surfaceless) Retorna true si se pudo inicializar un contexto gráfico.
bool trinity_renderer_init(int width, int height, bool headless);
void trinity_renderer_destroy();
void trinity_renderer_restore();

// Inicia el frame (glClear)
void trinity_renderer_frame_start();

// Finaliza el frame (eglSwapBuffers)
void trinity_renderer_frame_end();

// Cierra EGL
void trinity_renderer_close();

// Retorna false si se debe cerrar la aplicación
int trinity_renderer_poll_events();

// Primitive 2D Rendering (Overlay)
void trinity_renderer_draw_rect_2d(float x, float y, float w, float h, float r,
                                   float g, float b);

// Textured Rendering// Textures
unsigned int trinity_renderer_create_alpha_texture(int w, int h,
                                                   const void *pixels);
unsigned int trinity_renderer_create_texture(int w, int h, const void *pixels);
void trinity_renderer_draw_textured_rect_2d(float x, float y, float w, float h,
                                            unsigned int tex_id, float r,
                                            float g, float b, int is_font);

// Input State Query
void trinity_renderer_get_mouse(int *x, int *y, bool *clicked);

#endif // OPENGL_ES_BACKEND_H

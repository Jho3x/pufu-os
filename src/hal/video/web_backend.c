/*
 * web_backend.c
 * Pufu OS - Web Streaming Display Backend & Software Rasterizer
 *
 * Implements PufuFramebuffer AND the Trinity Renderer API via Software
 * Rasterization. This allows Pufu to run without an actual GPU context.
 */

#include "pufu/graphics.h"
#include "pufu/trinity.h" // Ensures we match the contract
// #include "opengl_es_backend.h" // We do NOT include this, we REPLACE it.

#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define WEB_PORT 8080
#define FB_WIDTH 800
#define FB_HEIGHT 600
#define MAX_TEXTURES 64

// --- Internal Structures ---
typedef struct {
  int w, h;
  unsigned char *pixels; // RGBA
  bool active;
} SoftTexture;

static SoftTexture g_textures[MAX_TEXTURES];
static int g_tex_count = 1; // Start at 1, 0 is invalid

// The Polymorphic Framebuffer Instance
static PufuFramebuffer g_web_fb = {0};
static int server_socket = -1;
static pthread_t server_thread_id;

// HTML Template
const char *HTML_VIEWER =
    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
    "<html><head><title>Pufu OS Remote</title>"
    "<meta http-equiv='refresh' content='1'>"
    "</head><body style='background:#111; color:#eee; text-align:center'>"
    "<h1>Pufu OS Remote Display</h1>"
    "<img src='/fb.ppm' width='800' height='600' style='border:2px solid "
    "#555'/>"
    "</body></html>";

// --- Server Stub ---
void send_ppm(int client_sock) {
  char header[64];
  snprintf(header, sizeof(header), "P6\n%d %d\n255\n", g_web_fb.width,
           g_web_fb.height);
  const char *http_head =
      "HTTP/1.1 200 OK\r\nContent-Type: image/x-portable-pixmap\r\n\r\n";
  send(client_sock, http_head, strlen(http_head), 0);
  send(client_sock, header, strlen(header), 0);

  unsigned char *rgb_buf = malloc(g_web_fb.width * g_web_fb.height * 3);
  unsigned char *src = (unsigned char *)g_web_fb.pixels;
  unsigned char *dst = rgb_buf;
  if (rgb_buf && src) {
    for (int i = 0; i < g_web_fb.width * g_web_fb.height; i++) {
      *dst++ = src[0];
      *dst++ = src[1];
      *dst++ = src[2];
      src += 4;
    }
    send(client_sock, rgb_buf, g_web_fb.width * g_web_fb.height * 3, 0);
    free(rgb_buf);
  }
}

void *server_thread_func(void *arg) {
  (void)arg;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  printf("[WebBackend] Server thread started on port %d\n", WEB_PORT);
  while (1) {
    int new_socket = accept(server_socket, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen);
    if (new_socket < 0)
      continue;
    char buffer[1024] = {0};
    read(new_socket, buffer, 1024);
    if (strstr(buffer, "GET /fb.ppm"))
      send_ppm(new_socket);
    else
      send(new_socket, HTML_VIEWER, strlen(HTML_VIEWER), 0);
    close(new_socket);
  }
  return NULL;
}

// --- Framebuffer Accessor ---
PufuFramebuffer *pufu_video_get_fb(void) {
  if (g_web_fb.pixels == NULL) {
    g_web_fb.width = FB_WIDTH;
    g_web_fb.height = FB_HEIGHT;
    g_web_fb.stride = FB_WIDTH * 4;
    g_web_fb.pixels = malloc(FB_WIDTH * FB_HEIGHT * 4);
    memset(g_web_fb.pixels, 0, FB_WIDTH * FB_HEIGHT * 4);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(WEB_PORT);
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) >=
        0) {
      listen(server_socket, 3);
      pthread_create(&server_thread_id, NULL, server_thread_func, NULL);
    }
  }
  return &g_web_fb;
}

// --- Trinity Renderer Implementation (Software Rasterizer) ---

bool trinity_renderer_init(int width, int height, bool headless) {
  (void)headless;
  g_web_fb.width = width;
  g_web_fb.height = height;
  pufu_video_get_fb(); // Ensure FB is ready
  printf("[SoftwareRenderer] Initialized at %dx%d\n", width, height);
  return true;
}

void trinity_renderer_frame_start() {
  // Clear screen
  pufu_graphics_clear(0.1f, 0.1f, 0.2f);
}

void trinity_renderer_frame_end() {
  // No-op, data is already in RAM for web server to read
}

void trinity_renderer_destroy() {
  if (g_web_fb.pixels)
    free(g_web_fb.pixels);
}

void trinity_renderer_restore() {} // No window to restore

int trinity_renderer_poll_events() {
  // Headless/Web: Always return 1 (Keep running), no inputs yet
  return 1;
}

void trinity_renderer_get_mouse(int *x, int *y, bool *clicked) {
  if (x)
    *x = 0;
  if (y)
    *y = 0;
  if (clicked)
    *clicked = false;
}

// -- Primitives --

void pufu_graphics_clear(float r, float g, float b) {
  if (!g_web_fb.pixels)
    return;
  unsigned char ir = (unsigned char)(r * 255);
  unsigned char ig = (unsigned char)(g * 255);
  unsigned char ib = (unsigned char)(b * 255);

  unsigned char *p = (unsigned char *)g_web_fb.pixels;
  int count = g_web_fb.width * g_web_fb.height;
  while (count--) {
    *p++ = ir;
    *p++ = ig;
    *p++ = ib;
    *p++ = 255;
  }
}

void trinity_renderer_draw_rect_2d(float x, float y, float w, float h, float r,
                                   float g, float b) {
  if (!g_web_fb.pixels)
    return;

  int ix = (int)x;
  int iy = (int)y;
  int iw = (int)w;
  int ih = (int)h;

  unsigned char ir = (unsigned char)(r * 255);
  unsigned char ig = (unsigned char)(g * 255);
  unsigned char ib = (unsigned char)(b * 255);

  // Clipping
  if (ix < 0) {
    iw += ix;
    ix = 0;
  }
  if (iy < 0) {
    ih += iy;
    iy = 0;
  }
  if (ix + iw > g_web_fb.width)
    iw = g_web_fb.width - ix;
  if (iy + ih > g_web_fb.height)
    ih = g_web_fb.height - iy;

  if (iw <= 0 || ih <= 0)
    return;

  unsigned char *row =
      (unsigned char *)g_web_fb.pixels + (iy * g_web_fb.stride) + (ix * 4);
  for (int j = 0; j < ih; j++) {
    unsigned char *p = row;
    for (int i = 0; i < iw; i++) {
      *p++ = ir;
      *p++ = ig;
      *p++ = ib;
      *p++ = 255; // Alpha
    }
    row += g_web_fb.stride;
  }
}

// -- Textures --

unsigned int trinity_renderer_create_texture(int w, int h, const void *pixels) {
  if (g_tex_count >= MAX_TEXTURES)
    return 0;
  int id = g_tex_count++;

  g_textures[id].w = w;
  g_textures[id].h = h;
  g_textures[id].pixels = malloc(w * h * 4);
  g_textures[id].active = true;

  if (g_textures[id].pixels) {
    memcpy(g_textures[id].pixels, pixels, w * h * 4);
  }
  return id;
}

unsigned int trinity_renderer_create_alpha_texture(int w, int h,
                                                   const void *pixels) {
  // Convert A8 -> RGBA
  unsigned char *rgba = malloc(w * h * 4);
  const unsigned char *src = (const unsigned char *)pixels;
  unsigned char *dst = rgba;

  for (int i = 0; i < w * h; i++) {
    *dst++ = 255;    // R
    *dst++ = 255;    // G
    *dst++ = 255;    // B
    *dst++ = *src++; // A
  }

  unsigned int id = trinity_renderer_create_texture(w, h, rgba);
  free(rgba);
  return id;
}

void trinity_renderer_draw_textured_rect_2d(float x, float y, float w, float h,
                                            unsigned int tex_id, float r,
                                            float g, float b, int is_font) {
  if (tex_id == 0 || tex_id >= (unsigned int)g_tex_count)
    return;
  SoftTexture *tex = &g_textures[tex_id];
  if (!tex->pixels)
    return;

  if (!g_web_fb.pixels)
    return;

  int ix = (int)x;
  int iy = (int)y;
  int iw = (int)w;
  int ih = (int)h;

  // Simplistic Scaling: Nearest Neighbor
  float u_step = (float)tex->w / (float)iw;
  float v_step = (float)tex->h / (float)ih;

  // Tint color
  int tint_r = (int)(r * 255);
  int tint_g = (int)(g * 255);
  int tint_b = (int)(b * 255);
  int is_alpha_mask = is_font; // 1 if we only care about alpha

  // Draw Loop with Clipping
  for (int j = 0; j < ih; j++) {
    int screen_y = iy + j;
    if (screen_y < 0 || screen_y >= g_web_fb.height)
      continue;

    int src_y = (int)(j * v_step);
    if (src_y >= tex->h)
      src_y = tex->h - 1;

    unsigned char *scanline =
        (unsigned char *)g_web_fb.pixels + (screen_y * g_web_fb.stride);

    for (int i = 0; i < iw; i++) {
      int screen_x = ix + i;
      if (screen_x < 0 || screen_x >= g_web_fb.width)
        continue;

      int src_x = (int)(i * u_step);
      if (src_x >= tex->w)
        src_x = tex->w - 1;

      // Sample Texture
      unsigned char *src_p = tex->pixels + (src_y * tex->w * 4) + (src_x * 4);
      unsigned char tex_r = src_p[0];
      unsigned char tex_g = src_p[1];
      unsigned char tex_b = src_p[2];
      unsigned char tex_a = src_p[3];

      // Apply Tint
      // If is_font (1), generic shader said: texture2D(tex) * u_color.
      // For font, texture is White + Alpha. u_color is the text color.
      // So result rgb = 1.0 * tint, alpha = tex_a.

      int final_r = (tex_r * tint_r) / 255;
      int final_g = (tex_g * tint_g) / 255;
      int final_b = (tex_b * tint_b) / 255;
      int final_a = tex_a;

      // Alpha Blending over logical 0,0,0 background?
      // dst = src * a + dst * (1-a)
      // Destination pixel
      unsigned char *dst_p = scanline + (screen_x * 4);
      unsigned char bg_r = dst_p[0];
      unsigned char bg_g = dst_p[1];
      unsigned char bg_b = dst_p[2];

      // Fast Blend
      int a = final_a;
      int inv_a = 255 - a;

      dst_p[0] = (unsigned char)((final_r * a + bg_r * inv_a) >> 8);
      dst_p[1] = (unsigned char)((final_g * a + bg_g * inv_a) >> 8);
      dst_p[2] = (unsigned char)((final_b * a + bg_b * inv_a) >> 8);
      dst_p[3] = 255; // Always opaque screen
    }
  }
}

// 3D Stubs (ToDo)
int pufu_graphics_load_model(const char *path) {
  (void)path;
  return 0;
}
void pufu_graphics_draw_model(int model_id, int texture_id, float x, float y,
                              float z, float scale, float ry) {
  (void)model_id;
  (void)texture_id;
  (void)x;
  (void)y;
  (void)z;
  (void)scale;
  (void)ry;
}
void pufu_create_primitive(int id) { (void)id; }
int pufu_graphics_load_texture(const char *path, int *w, int *h) {
  (void)path;
  (void)w;
  (void)h;
  return 0;
}
void pufu_graphics_draw_texture(int texture_id, float x, float y, float w,
                                float h) {
  (void)texture_id;
  (void)x;
  (void)y;
  (void)w;
  (void)h;
}

// --- Legacy Engine Stubs for Labeloid ---
void *engine_get_scene(void) { return NULL; }
void loader_parse_line(void *scene, char *line) {
  (void)scene;
  (void)line;
}
int scene_create_entity(void *scene) {
  (void)scene;
  return 0;
}

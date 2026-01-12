// #include "../../graphics/backend/opengl_es_backend.h"
#include "pufu/graphics.h"
#include "pufu/trinity.h"
#include "trinity_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STB Headers
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// NanoSVG Headers
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

// --- Public Globals (Defined in Internal Header) ---
// Defined in render or core?
// In trinity_internal.h they are extern.
// We should define them here if they are mostly used here?
// Core had init logic for font. But symbols must exist.
// Let's rely on Core defining globals?
// "extern" in header means someone MUST define it.
// I put `g_font_buffer` init in Core. So Core should define them?
// No, I accessed them in Core. If I defining them in Core, that works.
// Let's check Core content.
// Core defined: node_pool, node_count, renderer_active.
// Core DID NOT define g_font globals. It just accessed them.
// So Render must define them. Or Core.
// Core had `stbtt_InitFont`.
// I'll define g_font globals here in Render.

stbtt_fontinfo g_font_info;
unsigned char *g_font_buffer = NULL;
bool g_font_loaded = false;

// --- Bake Label ---
void bake_label(PayloadButton *btn) {
  if (!g_font_loaded || !btn)
    return;

  // Simple bitmap generation for string
  int w = 256;
  int h = 32;
  unsigned char *bitmap = calloc(1, w * h);

  float scale = stbtt_ScaleForPixelHeight(&g_font_info, 24); // 24pt
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&g_font_info, &ascent, &descent, &lineGap);

  int baseline = (int)(ascent * scale);

  btn->text_ascent = baseline;
  btn->text_descent = (int)(descent * scale);
  btn->text_baseline = baseline;

  int x = 0;

  for (int i = 0; i < (int)strlen(btn->label); ++i) {
    int ax, lsb;
    stbtt_GetCodepointHMetrics(&g_font_info, btn->label[i], &ax, &lsb);

    int c_x1, c_y1, c_x2, c_y2;
    stbtt_GetCodepointBitmapBox(&g_font_info, btn->label[i], scale, scale,
                                &c_x1, &c_y1, &c_x2, &c_y2);

    int y = baseline + c_y1;
    (void)y;

    int bw, bh, xoff, yoff;
    unsigned char *glyph = stbtt_GetCodepointBitmap(
        &g_font_info, 0, scale, btn->label[i], &bw, &bh, &xoff, &yoff);

    int out_x = x + xoff;
    int out_y = baseline + yoff;

    for (int j = 0; j < bh; ++j) {
      for (int k = 0; k < bw; ++k) {
        if (out_x + k < w && out_y + j < h && out_y + j >= 0) {
          bitmap[(out_y + j) * w + (out_x + k)] = glyph[j * bw + k];
        }
      }
    }
    stbtt_FreeBitmap(glyph, NULL);

    x += (int)(ax * scale);
    if (i < (int)strlen(btn->label) - 1) {
      int kern = stbtt_GetCodepointKernAdvance(&g_font_info, btn->label[i],
                                               btn->label[i + 1]);
      x += (int)(kern * scale);
    }
  }

  if (btn->text_texture_id) {
    // TODO: Delete old
  }

  btn->text_texture_id = trinity_renderer_create_alpha_texture(w, h, bitmap);
  btn->text_w = x;
  btn->text_h = h;

  free(bitmap);
}

// --- SVG Loading helper ---
// Returns texture ID, or 0 on failure. Sets w/h.
int trinity_load_svg(const char *filename, int *out_w, int *out_h) {
  if (!filename)
    return 0;

  // Parse SVG
  NSVGimage *image = nsvgParseFromFile(filename, "px", 96.0f);
  if (!image) {
    printf("[TRINITY] Nanosvg failed to parse '%s'\n", filename);
    return 0;
  }

  int w = (int)image->width;
  int h = (int)image->height;

  // Create rasterizer
  NSVGrasterizer *rast = nsvgCreateRasterizer();
  if (!rast) {
    printf("[TRINITY] Failed to create SVG rasterizer.\n");
    nsvgDelete(image);
    return 0;
  }

  // Rasterize
  // Nanosvg produces RGBA buffer (4 bytes per pixel)
  // IMPORTANT: It seems it might be premultiplied alpha? Or not.
  // Stb_image usually gives straight RGBA.
  unsigned char *img = malloc(w * h * 4);
  if (!img) {
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);
    return 0;
  }

  printf("[TRINITY] Rasterizing SVG '%s' (%dx%d)...\n", filename, w, h);

  // Rasterize to buffer
  // nsvgRasterize(rast, image, tx, ty, scale, dst, w, h, stride)
  nsvgRasterize(rast, image, 0, 0, 1.0f, img, w, h, w * 4);

  // Upload to GPU
  int tex_id = 0;
  if (renderer_active) {
    // We reuse create_texture from regular images (RGBA)
    tex_id = trinity_renderer_create_texture(w, h, img);
  }

  *out_w = w;
  *out_h = h;

  free(img);
  nsvgDeleteRasterizer(rast);
  nsvgDelete(image);

  return tex_id;
}

// --- Render Loop ---
void trinity_render_frame(void) {
  // Painter's Algorithm
  trinity_renderer_frame_start();

  for (int i = 0; i < node_count; i++) {
    Node *n = &node_pool[i];
    if (!n->data_ptr)
      continue;
    if (!(n->flags & NODE_FLAG_VISIBLE))
      continue;

    switch (n->data_type) {
    case DATA_UI_IMAGE: {
      PayloadImage *img = (PayloadImage *)n->data_ptr;
      if (img->texture_id > 0) {
        trinity_renderer_draw_textured_rect_2d(img->x, img->y, img->width,
                                               img->height, img->texture_id, 1,
                                               1, 1, 0);
      }
      break;
    }
    case DATA_FRAME: {
      PayloadFrame *win = (PayloadFrame *)n->data_ptr;
      // Background
      trinity_renderer_draw_rect_2d(win->x, win->y, win->width, win->height,
                                    win->bg_color.x, win->bg_color.y,
                                    win->bg_color.z);
      // Header
      if (win->movable) {
        trinity_renderer_draw_rect_2d(
            win->x, win->y, win->width, 30, win->bg_color.x * 0.8f,
            win->bg_color.y * 0.8f, win->bg_color.z * 0.8f);
      }
      break;
    }
    case DATA_UI_BUTTON: {
      PayloadButton *btn = (PayloadButton *)n->data_ptr;
      vec3 c = btn->is_pressed ? btn->color_on : btn->color_off;
      trinity_renderer_draw_rect_2d(btn->x, btn->y, btn->width, btn->height,
                                    c.x, c.y, c.z);

      if (btn->text_texture_id > 0) {
        if (btn->icon_texture_id > 0) {
          float icon_x = btn->x + (btn->width - btn->icon_w) * 0.5f;
          float icon_y = btn->y + (btn->height - btn->icon_h) * 0.5f;
          trinity_renderer_draw_textured_rect_2d(
              icon_x, icon_y, btn->icon_w, btn->icon_h, btn->icon_texture_id, 1,
              1, 1, 0);
        }
        float tx = btn->x + (btn->width - btn->text_w) * 0.5f;
        int font_h = btn->text_ascent - btn->text_descent;
        float ty = btn->y + (btn->height - font_h) * 0.5f;
        trinity_renderer_draw_textured_rect_2d(
            tx, ty, 256, 32, btn->text_texture_id, 1, 1, 1, 1);
      }
      break;
    }
    default:
      break;
    }
  }

  trinity_renderer_frame_end();
}

// --- Lifecycle Support ---
void trinity_prepare(void) {
  // 1. Scan for Window Node (legacy check for resolution)
  bool found_window = false;
  int win_w = 800;
  int win_h = 600;

  for (int i = 0; i < node_count; i++) {
    if (node_pool[i].type == NODE_WINDOW &&
        node_pool[i].data_type == DATA_WINDOW && node_pool[i].data_ptr) {
      found_window = true;
      PayloadWindow *win = (PayloadWindow *)node_pool[i].data_ptr;
      win_w = win->width;
      win_h = win->height;
      break;
    }
  }

  // 2. Init Backend
  if (!renderer_active) {
    renderer_active = trinity_renderer_init(
        win_w > 0 ? win_w : 800, win_h > 0 ? win_h : 600, !found_window);
  } else {
    if (found_window) {
      trinity_renderer_restore();
    }
  }

  // 3. Deferred Loading
  if (renderer_active) {
    for (int i = 0; i < node_count; i++) {
      if (node_pool[i].data_type == DATA_UI_IMAGE) {
        PayloadImage *img = (PayloadImage *)node_pool[i].data_ptr;
        if (img->texture_id == 0 && strlen(img->path) > 0) {
          printf("[TRINITY] Diffused Loading for Image Node '%s'...\n",
                 node_pool[i].name);

          int w = 0, h = 0, ch;
          int tex_id = 0;

          const char *ext = strrchr(img->path, '.');
          if (ext && strcmp(ext, ".svg") == 0) {
            tex_id = trinity_load_svg(img->path, &w, &h);
          } else {
            stbi_set_flip_vertically_on_load(1);
            unsigned char *pixels = stbi_load(img->path, &w, &h, &ch, 4);
            if (pixels) {
              tex_id = trinity_renderer_create_texture(w, h, pixels);
              stbi_image_free(pixels);
            }
          }

          if (tex_id > 0) {
            img->texture_id = tex_id;
            printf("[TRINITY] Loaded Deferred Image '%s' (%dx%d)\n", img->path,
                   w, h);
          } else {
            printf("[TRINITY] Failed to load deferred image: %s\n", img->path);
          }
        }
      }
    }
  }
}

void trinity_render_boot_sequence(void) {
  trinity_renderer_frame_start();
  pufu_graphics_clear(0.0f, 0.0f, 0.0f); // Black background

  // Retroactive Logs
  printf("STATUS OK\n");

  FILE *f = fopen("welcome.pufu", "r");
  if (f) {
    char line[256];
    while (fgets(line, sizeof(line), f)) {
      if (strstr(line, "syscall") == NULL && strstr(line, "#") == NULL) {
        printf("%s", line);
      }
    }
    fclose(f);
  }

  trinity_renderer_frame_end();
}

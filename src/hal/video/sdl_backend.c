#include "opengl_es_backend.h"
#include "pufu/graphics.h"
#include "pufu/trinity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

// X11 includes (assuming Linux environment)
#include <X11/Xlib.h>
#include <X11/keysym.h>

// ... (existing code)

// Global Atom for Close
static Atom wm_delete_window;

// Global EGL State
static EGLDisplay egl_display = EGL_NO_DISPLAY;
static EGLContext egl_context = EGL_NO_CONTEXT;
static EGLSurface egl_surface = EGL_NO_SURFACE;
static Display *x_display = NULL;
static Window x_window = 0;
static int g_win_width = 800;
static int g_win_height = 600;

// Input State
static int mouse_x = 0;
static int mouse_y = 0;
static bool mouse_clicked = false;

// ... (in init)
// ... (in init)

// Returns: 1 (Continue), 0 (Close), 2 (Background)
int trinity_renderer_poll_events() {
  if (!x_display || !x_window)
    return 1; // Headless always true

  // Reset per-frame input states
  mouse_clicked = false;

  while (XPending(x_display) > 0) {
    XEvent event;
    XNextEvent(x_display, &event);

    // printf("[DEBUG] Event Type: %d\n", event.type);

    if (event.type == ClientMessage) {
      if ((Atom)event.xclient.data.l[0] == wm_delete_window) {
        return 0; // Close requested
      }
    } else if (event.type == KeyPress) {
      KeySym key = XLookupKeysym(&event.xkey, 0);
      // printf("[DEBUG] KeyPress: %lu (State: %u)\n", key, event.xkey.state);

      if (key == XK_Escape) {
        return 0; // ESC pressed
      }
      if (key == XK_p && (event.xkey.state & (ControlMask | Mod1Mask))) {
        printf("[RENDERER] Background Request Detected (Ctrl+Alt+P).\n");
        XIconifyWindow(x_display, x_window, DefaultScreen(x_display));
        return 2; // Background
      }
    } else if (event.type == DestroyNotify) {
      printf("[RENDERER] DestroyNotify received.\n");
      return 0;
    } else if (event.type == ButtonPress) {
      if (event.xbutton.button == Button1) { // Left Click
        mouse_clicked = true;
        mouse_x = event.xbutton.x;
        mouse_y = event.xbutton.y;
        trinity_queue_event(4, event.xbutton.x, event.xbutton.y, -1);
      }
    } else if (event.type == ButtonRelease) {
      if (event.xbutton.button == Button1) {
        trinity_queue_event(5, event.xbutton.x, event.xbutton.y, -1);
      }
    } else if (event.type == MotionNotify) {
      mouse_x = event.xmotion.x;
      mouse_y = event.xmotion.y;
      trinity_queue_event(6, event.xmotion.x, event.xmotion.y, -1);
    }
  }
  return 1;
}

// Declarations moved to top

bool trinity_renderer_init(int width, int height, bool headless) {
  g_win_width = width;
  g_win_height = height;
  printf("[RENDERER] Initializing EGL (Headless=%d, Size=%dx%d)...\n", headless,
         width, height);

  // 1. Get Native Display (X11)
  if (!headless) {
    x_display = XOpenDisplay(NULL);
    if (x_display == NULL) {
      printf(
          "[RENDERER] Error: Cannot open X Display. Fallback to Headless?\n");
      // Could fallback, but for now strict fail if window requested
      // headless = true;
      return false;
    }
  }

  // 2. Initialize EGL
  egl_display = eglGetDisplay((EGLNativeDisplayType)x_display);
  if (egl_display == EGL_NO_DISPLAY) {
    printf("[RENDERER] Error: eglGetDisplay failed.\n");
    return false;
  }

  EGLint major, minor;
  if (!eglInitialize(egl_display, &major, &minor)) {
    printf("[RENDERER] Error: eglInitialize failed.\n");
    return false;
  }
  printf("[RENDERER] EGL Version: %d.%d\n", major, minor);

  // 3. Choose Config
  EGLint attribs[] = {EGL_SURFACE_TYPE,
                      (headless ? EGL_PBUFFER_BIT : EGL_WINDOW_BIT),
                      EGL_RENDERABLE_TYPE,
                      EGL_OPENGL_ES2_BIT,
                      EGL_RED_SIZE,
                      8,
                      EGL_GREEN_SIZE,
                      8,
                      EGL_BLUE_SIZE,
                      8,
                      EGL_DEPTH_SIZE,
                      24,
                      EGL_NONE};

  EGLConfig config;
  EGLint num_configs;
  if (!eglChooseConfig(egl_display, attribs, &config, 1, &num_configs) ||
      num_configs < 1) {
    printf("[RENDERER] Error: No suitable EGL config found.\n");
    return false;
  }

  // 4. Create Surface (Window or Pbuffer)
  if (headless) {
    EGLint pbuffer_attribs[] = {
        EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE,
    };
    egl_surface = eglCreatePbufferSurface(egl_display, config, pbuffer_attribs);
  } else {
    // Create X11 Window
    Window root = DefaultRootWindow(x_display);
    XSetWindowAttributes swa;
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask |
                     ButtonPressMask | ButtonReleaseMask;

    x_window =
        XCreateWindow(x_display, root, 0, 0, width, height, 0, CopyFromParent,
                      InputOutput, CopyFromParent, CWEventMask, &swa);

    // FIX: Set Atom for Close Event
    wm_delete_window = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x_display, x_window, &wm_delete_window, 1);

    XMapWindow(x_display, x_window);
    XStoreName(x_display, x_window, "Trinity Engine");

    egl_surface = eglCreateWindowSurface(egl_display, config,
                                         (EGLNativeWindowType)x_window, NULL);
  }

  if (egl_surface == EGL_NO_SURFACE) {
    printf("[RENDERER] Error: Failed to create EGL Surface.\n");
    return false;
  }

  // 5. Create Context
  EGLint ctx_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  egl_context =
      eglCreateContext(egl_display, config, EGL_NO_CONTEXT, ctx_attribs);

  if (egl_context == EGL_NO_CONTEXT) {
    printf("[RENDERER] Error: Failed to create EGL Context.\n");
    return false;
  }

  // 6. Make Current
  if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
    printf("[RENDERER] Error: eglMakeCurrent failed (Error: 0x%x).\n",
           eglGetError());
    return false;
  }

  // printf("[RENDERER] Context Made Current.\n");
  // glGetString queries removed for release silence

  return true;
}

void trinity_renderer_frame_start() {
  glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Dark Blueish
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void trinity_renderer_frame_end() {
  if (egl_display && egl_surface) {
    eglSwapBuffers(egl_display, egl_surface);
  }
}

void trinity_renderer_restore() {
  if (x_display && x_window) {
    XMapWindow(x_display, x_window);
    XRaiseWindow(x_display, x_window);
    printf("[RENDERER] Window Restored.\n");
  }
}
void trinity_renderer_destroy() {
  if (egl_display != EGL_NO_DISPLAY) {
    eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (egl_context != EGL_NO_CONTEXT) {
      eglDestroyContext(egl_display, egl_context);
      egl_context = EGL_NO_CONTEXT;
    }
    if (egl_surface != EGL_NO_SURFACE) {
      eglDestroySurface(egl_display, egl_surface);
      egl_surface = EGL_NO_SURFACE;
    }
    eglTerminate(egl_display);
    egl_display = EGL_NO_DISPLAY;
  }

  if (x_display) {
    XCloseDisplay(x_display);
    x_display = NULL;
  }

  printf("[RENDERER] EGL Shutdown.\n");
}

void trinity_renderer_draw_rect_2d(float x, float y, float w, float h, float r,
                                   float g, float b) {
  // Hack: Use Scissor Test to draw solid blocks without shaders/VBOs
  if (!egl_display)
    return;

  // Need current window height for Y-flip
  // Assuming 800x600 or querying? We don't store height globally here yet
  // except in init arg. Let's assume 600 or query later. For now, hardcode
  // 600 or pass it. Better: Query current surface?  // Let's store win_height
  // in global. Fallback: Just draw.

  int win_h = g_win_height;

  glEnable(GL_SCISSOR_TEST);
  // glScissor takes (lower-left-x, lower-left-y, width, height)
  // Our UI coords are (top-left-x, top-left-y).
  // So scissor_y = win_h - y - h.
  glScissor((int)x, win_h - (int)y - (int)h, (int)w, (int)h);
  glClearColor(r, g, b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_SCISSOR_TEST);
}

// --- Shader Support ---
static GLuint g_program = 0;
static GLint g_u_color = -1;
static GLint g_u_texture = -1;
static GLint g_a_pos = -1;
static GLint g_a_uv = -1;
static GLint g_u_mask_mode = -1;

// Simple Vertex Shader
static const char *vs_src = "attribute vec2 a_pos;\n"
                            "attribute vec2 a_uv;\n"
                            "varying vec2 v_uv;\n"
                            "void main() {\n"
                            "  gl_Position = vec4(a_pos, 0.0, 1.0);\n"
                            "  v_uv = a_uv;\n"
                            "}\n";

// Universal Fragment Shader (Alpha or RGBA)
static const char *fs_src =
    "precision mediump float;\n"
    "uniform vec4 u_color;\n"
    "uniform sampler2D u_texture;\n"
    "uniform float u_mask_mode;\n" // 1.0 = Alpha Mask (Text), 0.0 = Start
                                   // Texture (Icon)
    "varying vec2 v_uv;\n"
    "void main() {\n"
    "  // Universal Render: Texture * Tint Color\n"
    "  // Textures are RGBA (Icons) or White+Alpha (Text)\n"
    "  gl_FragColor = texture2D(u_texture, v_uv) * u_color;\n"
    "}\n";

static GLuint compile_shader(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  // Check error omitted for brevity
  return shader;
}

static void init_shaders() {
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);
  g_program = glCreateProgram();
  glAttachShader(g_program, vs);
  glAttachShader(g_program, fs);
  glLinkProgram(g_program);

  g_a_pos = glGetAttribLocation(g_program, "a_pos");
  g_a_uv = glGetAttribLocation(g_program, "a_uv");
  g_u_color = glGetUniformLocation(g_program, "u_color");
  g_u_texture = glGetUniformLocation(g_program, "u_texture");
  g_u_mask_mode = glGetUniformLocation(g_program, "u_mask_mode");
}

// Use GL_ALPHA format for single channel (A8) textures
// Use GL_RGBA for robust compatibility (GL_ALPHA can be flaky on some drivers)
unsigned int trinity_renderer_create_alpha_texture(int w, int h,
                                                   const void *pixels) {
  if (!egl_display)
    return 0;

  // Expand 1-byte Alpha to 4-byte RGBA (255, 255, 255, A)
  unsigned char *rgba = malloc(w * h * 4);
  const unsigned char *src = (const unsigned char *)pixels;
  for (int i = 0; i < w * h; i++) {
    rgba[i * 4 + 0] = 255;
    rgba[i * 4 + 1] = 255;
    rgba[i * 4 + 2] = 255;
    rgba[i * 4 + 3] = src[i];
  }

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Upload as RGBA
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               rgba);
  glBindTexture(GL_TEXTURE_2D, 0);

  free(rgba);
  return tex;
}

unsigned int trinity_renderer_create_texture(int w, int h, const void *pixels) {
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  // Set parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Upload RGBA
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               pixels);

  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

void trinity_renderer_draw_textured_rect_2d(float x, float y, float w, float h,
                                            unsigned int tex_id, float r,
                                            float g, float b, int is_font) {
  if (!g_program)
    init_shaders();

  glUseProgram(g_program);
  glUniform1f(g_u_mask_mode, (float)is_font);

  // Normalize coordinates to -1..1
  // Screen: (0,0) top-left -> (w,h) bottom-right
  // NDC: (-1, 1) top-left -> (1, -1) bottom-right

  float ndc_x = (x / g_win_width) * 2.0f - 1.0f;
  float ndc_y = 1.0f - (y / g_win_height) * 2.0f;
  float ndc_w = (w / g_win_width) * 2.0f;
  float ndc_h = (h / g_win_height) * 2.0f;

  // Quad Vertices (Triangle Strip)
  // x, y
  float verts[] = {
      ndc_x,         ndc_y,         // TL
      ndc_x,         ndc_y - ndc_h, // BL
      ndc_x + ndc_w, ndc_y,         // TR
      ndc_x + ndc_w, ndc_y - ndc_h  // BR
  };

  // UVs
  float uvs[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

  glEnableVertexAttribArray(g_a_pos);
  glVertexAttribPointer(g_a_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);

  glEnableVertexAttribArray(g_a_uv);
  glVertexAttribPointer(g_a_uv, 2, GL_FLOAT, GL_FALSE, 0, uvs);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glUniform1i(g_u_texture, 0);

  glUniform4f(g_u_color, r, g, b, 1.0f);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisable(GL_BLEND);
}

void trinity_renderer_get_mouse(int *x, int *y, bool *clicked) {
  if (x)
    *x = mouse_x;
  if (y)
    *y = mouse_y;
  if (clicked)
    *clicked = mouse_clicked;
}

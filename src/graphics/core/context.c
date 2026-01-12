#include "pufu/graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// Global State (Internal to graphics module)
Display *x_display = NULL;
Window x_window;
EGLDisplay egl_display;
EGLContext egl_context;
EGLSurface egl_surface;
int screen_width, screen_height;

// Headless Flag
static int is_headless = 0;

void pufu_graphics_set_headless(int h) { is_headless = h; }

// Forward declarations for other modules
void pufu_renderer_init(void);
void pufu_renderer_cleanup(void);

int pufu_graphics_init(int width, int height, const char *title) {
  if (is_headless) {
    printf("Trinity: Headless Mode Enabled. Skipping Window Creation.\n");
    return 0; // Success (Simulated)
  }

  screen_width = width;
  screen_height = height;
  printf("Trinity: Initializing Graphics (X11 + EGL) %dx%d...\n", width,
         height);

  // 1. Open X11 Display
  x_display = XOpenDisplay(NULL);
  if (!x_display) {
    printf("Trinity Error: Cannot open X display\n");
    return -1;
  }

  // 2. Initialize EGL
  egl_display = eglGetDisplay((EGLNativeDisplayType)x_display);
  if (egl_display == EGL_NO_DISPLAY) {
    printf("Trinity Error: eglGetDisplay failed\n");
    return -1;
  }

  EGLint major, minor;
  if (!eglInitialize(egl_display, &major, &minor)) {
    printf("Trinity Error: eglInitialize failed\n");
    return -1;
  }
  printf("Trinity: EGL v%d.%d initialized\n", major, minor);

  // 3. Choose EGL Config
  const EGLint config_attribs[] = {EGL_SURFACE_TYPE,
                                   EGL_WINDOW_BIT,
                                   EGL_RED_SIZE,
                                   8,
                                   EGL_GREEN_SIZE,
                                   8,
                                   EGL_BLUE_SIZE,
                                   8,
                                   EGL_RENDERABLE_TYPE,
                                   EGL_OPENGL_ES3_BIT,
                                   EGL_NONE};

  EGLConfig config;
  EGLint num_configs;
  if (!eglChooseConfig(egl_display, config_attribs, &config, 1, &num_configs) ||
      num_configs < 1) {
    printf("Trinity Error: eglChooseConfig failed\n");
    return -1;
  }

  // 4. Get Visual from EGL Config
  EGLint visual_id;
  if (!eglGetConfigAttrib(egl_display, config, EGL_NATIVE_VISUAL_ID,
                          &visual_id)) {
    printf("Trinity Error: eglGetConfigAttrib failed\n");
    return -1;
  }

  // 5. Create X11 Window with compatible Visual
  XVisualInfo visTemplate;
  visTemplate.visualid = visual_id;
  int num_visuals;
  XVisualInfo *visInfo =
      XGetVisualInfo(x_display, VisualIDMask, &visTemplate, &num_visuals);
  if (!visInfo) {
    printf("Trinity Error: XGetVisualInfo failed\n");
    return -1;
  }

  XSetWindowAttributes attr;
  attr.colormap = XCreateColormap(
      x_display, RootWindow(x_display, DefaultScreen(x_display)),
      visInfo->visual, AllocNone);
  attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;

  x_window =
      XCreateWindow(x_display, RootWindow(x_display, DefaultScreen(x_display)),
                    0, 0, width, height, 0, visInfo->depth, InputOutput,
                    visInfo->visual, CWColormap | CWEventMask, &attr);

  XStoreName(x_display, x_window, title);
  XMapWindow(x_display, x_window);
  XFree(visInfo);

  // 6. Create EGL Context
  const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  egl_context =
      eglCreateContext(egl_display, config, EGL_NO_CONTEXT, context_attribs);
  if (egl_context == EGL_NO_CONTEXT) {
    printf("Trinity Error: eglCreateContext failed\n");
    return -1;
  }

  // 7. Create EGL Surface
  egl_surface = eglCreateWindowSurface(egl_display, config,
                                       (EGLNativeWindowType)x_window, NULL);
  if (egl_surface == EGL_NO_SURFACE) {
    printf("Trinity Error: eglCreateWindowSurface failed\n");
    return -1;
  }

  // 8. Make Current
  if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
    printf("Trinity Error: eglMakeCurrent failed\n");
    return -1;
  }

  printf("Trinity: Graphics System Ready (OpenGL ES 3.0)\n");

  // Initialize Renderer (Shaders, Buffers)
  pufu_renderer_init();

  return 0;
}

void pufu_graphics_swap(void) { eglSwapBuffers(egl_display, egl_surface); }

#include <X11/keysym.h>

int pufu_graphics_poll(void) {
  if (!x_display)
    return 0;

  XEvent event;
  while (XPending(x_display)) {
    XNextEvent(x_display, &event);

    if (event.type == DestroyNotify) {
      return 0;
    }

    if (event.type == KeyPress) {
      KeySym key = XLookupKeysym(&event.xkey, 0);
      // Check for Ctrl + Alt + P
      // ControlMask = Ctrl
      // Mod1Mask = Alt (usually)
      if ((event.xkey.state & ControlMask) && (event.xkey.state & Mod1Mask)) {
        if (key == XK_p || key == XK_P) {
          printf("Trinity: Hotkey (Ctrl+Alt+P) detected. Closing...\n");
          return 0;
        }
      }
    }
  }
  return 1;
}

void pufu_graphics_cleanup(void) {
  pufu_renderer_cleanup();

  if (egl_display != EGL_NO_DISPLAY) {
    eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (egl_context != EGL_NO_CONTEXT)
      eglDestroyContext(egl_display, egl_context);
    if (egl_surface != EGL_NO_SURFACE)
      eglDestroySurface(egl_display, egl_surface);
    eglTerminate(egl_display);
  }
  if (x_display) {
    XDestroyWindow(x_display, x_window);
    XCloseDisplay(x_display);
  }
  printf("Trinity: Graphics Cleanup Done.\n");
}

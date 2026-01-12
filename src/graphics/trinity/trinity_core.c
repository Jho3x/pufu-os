#include "pufu/graphics.h"
// #include "../renderer/opengl_es_backend.h" -> Moved to backend, but we should
// use abstractions if possible. For now, let's fix path relative to include or
// just use "pufu/graphics.h" if it exposes backend? Actually, let's look for
// where opengl_es_backend.h is.
// #include "../../graphics/backend/opengl_es_backend.h"
#include "pufu/trinity.h"
#include "trinity_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// --- Global State Definitions ---
Node node_pool[MAX_NODES];
int node_count = 0;
bool renderer_active = false;

// --- Init ---
void trinity_init() {
  printf("[TRINITY] Initializing Core (Universal Node Architecture)...\n");
  memset(node_pool, 0, sizeof(node_pool));
  node_count = 0;

  // Font loading delegated to render module init?
  // Or just do it here as part of global init.
  // We need file access.
  FILE *f = fopen("media/fonts/static/Merriweather_24pt-Regular.ttf", "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    g_font_buffer = malloc(size);
    fread(g_font_buffer, 1, size, f);
    fclose(f);

    if (stbtt_InitFont(&g_font_info, g_font_buffer, 0)) {
      g_font_loaded = true;
      printf("[TRINITY] Font Loaded Successfully.\n");
    }
  } else {
    printf("[TRINITY] ERROR: Font not found.\n");
  }
}

void trinity_shutdown() {
  printf("[TRINITY] Shutting down. Freeing payloads...\n");

  trinity_renderer_destroy();
  renderer_active = false;

  for (int i = 0; i < node_count; i++) {
    if (node_pool[i].data_ptr) {
      free(node_pool[i].data_ptr);
      node_pool[i].data_ptr = NULL;
    }
  }
  node_count = 0;
  if (g_font_buffer)
    free(g_font_buffer);
}

// --- Main Loop Orchestration ---
static int g_frame = 0;

int trinity_step(void) {
  if (!renderer_active)
    return 1;

  // 1. Poll Events (Delegated to Events/Backend)
  // We use the backend directly here or wrapper?
  // Let's use backend directly then queue via wrapper.
  // Actually original code called trinity_renderer_poll_events which calls
  // backend. We should keep that logic clean. But wait, Poll Events logic was
  // inside trinity_step originally? Yes: `int status =
  // trinity_renderer_poll_events();`

  int status = trinity_renderer_poll_events();
  if (status == 0)
    return 0; // Close
  if (status == 2)
    return 2; // BG

  // 2. Logic Update (Mouse clicks etc)
  // This logic should probably be in events module? "process_input()"
  // Let's abstract it: trinity_process_input();
  // But for now, let's keep it here or call a helper in events.c?
  // Let's move the mouse toggle logic to `trinity_events.c` ->
  // `trinity_update_interaction()`. I'll declare `void
  // trinity_update_interaction(void);` in internal.h.

  // Actually, I'll inline it or move it later. Let's move it to
  // `trinity_render.c`? No, events. I will add `trinity_update_interaction` to
  // internal header next step. For now, I will assume it exists.

  // 2. Logic Update
  trinity_update_interaction();

  // 3. Render
  trinity_render_frame();

  usleep(16000);

  if (g_frame % 60 == 0) {
    if (renderer_active)
      printf("[TRINITY] Frame %d: Simulating %d nodes...\n", g_frame,
             node_count);
  }
  g_frame++;
  return 1;
}

int trinity_run(NodeID gear_node) {
  (void)gear_node;
  trinity_prepare(); // Defined in render module

  bool running = true;
  int exit_reason = TRINITY_EXIT_OK;
  while (running) {
    int ret = trinity_step();
    if (ret != 1) {
      running = false;
      exit_reason = (ret == 2) ? TRINITY_EXIT_BG : TRINITY_EXIT_OK;
    }
  }

  if (renderer_active && exit_reason != TRINITY_EXIT_BG) {
    trinity_shutdown(); // Shutdown on exit? Original code did destroy but kept
                        // pools?
    // Original: trinity_renderer_destroy(); renderer_active = false;
    // Calling trinity_shutdown clears pools too.
    // Original only destroyed renderer if not BG.
    trinity_renderer_destroy();
    renderer_active = false;
  }
  return exit_reason;
}

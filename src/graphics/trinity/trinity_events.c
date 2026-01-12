// #include "../../graphics/backend/opengl_es_backend.h"
#include "pufu/graphics.h"
#include "pufu/trinity.h"
#include "trinity_internal.h"
#include <stdio.h>
#include <string.h>

// --- Global Event Queue ---
TrinityEvent g_event_queue[EVENT_QUEUE_SIZE];
int g_event_head = 0;
int g_event_tail = 0;
int g_event_count = 0;

// Forward Decl for internal hit test
// NodeID trinity_get_node_at(int x, int y);
// Already in header? No, it's public. Wait, it wasn't in internal header.
// It SHOULD be if used by render logic?
// No, it's mostly internal to events or window manager (via kernel).
// But `trinity_queue_event` uses it. So we need it defined here.

NodeID trinity_get_node_at(int x, int y) {
  // Reverse Painter's Algorithm (Top-most first)
  for (int i = node_count - 1; i >= 0; i--) {
    Node *n = &node_pool[i];
    if (!n->data_ptr || !(n->flags & NODE_FLAG_VISIBLE))
      continue;

    float nx = 0, ny = 0, nw = 0, nh = 0;
    bool has_hit = false;

    switch (n->data_type) {
    case DATA_FRAME: {
      PayloadFrame *win = (PayloadFrame *)n->data_ptr;
      nx = win->x;
      ny = win->y;
      nw = win->width;
      nh = win->height;
      has_hit = true;
      break;
    }
    case DATA_UI_BUTTON: {
      PayloadButton *btn = (PayloadButton *)n->data_ptr;
      nx = btn->x;
      ny = btn->y;
      nw = btn->width;
      nh = btn->height;
      has_hit = true;
      break;
    }
    case DATA_UI_IMAGE: {
      PayloadImage *img = (PayloadImage *)n->data_ptr;
      nx = img->x;
      ny = img->y;
      nw = img->width;
      nh = img->height;
      has_hit = true;
      break;
    }
    default:
      break;
    }

    if (has_hit) {
      if (x >= nx && x <= nx + nw && y >= ny && y <= ny + nh) {
        return n->id;
      }
    }
  }
  return -1;
}

void trinity_queue_event(int type, int x, int y, NodeID target) {
  if (g_event_count >= EVENT_QUEUE_SIZE)
    return; // Drop if full

  // Auto-Hit Test for Inputs
  if (target == -1 && (type >= 4 && type <= 6)) {
    target = trinity_get_node_at(x, y);
  }

  TrinityEvent *e = &g_event_queue[g_event_head];
  e->type = type;
  e->x = x;
  e->y = y;
  e->target_id = target;

  g_event_head = (g_event_head + 1) % EVENT_QUEUE_SIZE;
  g_event_count++;
}

bool trinity_dequeue_event(TrinityEvent *out_event) {
  if (g_event_count == 0)
    return false;

  *out_event = g_event_queue[g_event_tail];
  g_event_tail = (g_event_tail + 1) % EVENT_QUEUE_SIZE;
  g_event_count--;
  return true;
}

// --- Interaction Logic (Extract from Step) ---
void trinity_update_interaction(void) {
  // Logic from old trinity_step
  // Actually, wait, this logic was inside trinity_step in implementation.c
  // It polls backend mouse state (which is separate from Event Queue?)
  // Yes, backend polling happened, then event queuing happened inside backend
  // (via callback?) No, `trinity_renderer_poll_events` in backend.c calls
  // `trinity_queue_event`.

  // This function is for "Immediate Mode" interactions (like Button Toggle
  // visualization). Ideally this should consume the event queue? But the
  // original code polled GetMouse directly.

  int mx, my;
  bool clk;
  static bool prev_clk = false;
  trinity_renderer_get_mouse(&mx, &my, &clk);
  bool click_trigger = clk && !prev_clk;

  for (int i = 0; i < node_count; i++) {
    Node *n = &node_pool[i];
    if (!n->data_ptr)
      continue;

    // Button Logic
    if (n->type == NODE_UI_BUTTON && n->data_type == DATA_UI_BUTTON) {
      PayloadButton *btn = (PayloadButton *)n->data_ptr;

      // Keep bake logic? It's render related but state update.
      // Moving bake check here seems fine.
      // Actually bake_label call.

      if (click_trigger) {
        if (mx >= btn->x && mx <= btn->x + btn->width && my >= btn->y &&
            my <= btn->y + btn->height) {
          btn->is_pressed = !btn->is_pressed; // Toggle
          printf("[TRINITY] Button '%s' Toggled. New State: %s\n", n->name,
                 btn->is_pressed ? "PRESSED (Dark)" : "RELEASED (Light)");
        }
      }
    }
  }
  prev_clk = clk;
}

// --- Binding Logic ---
void trinity_bind_event(NodeID id, int event_type, const char *cmd) {
  (void)event_type;
  Node *n = get_node(id);
  if (!n)
    return;

  if (n->data_type == DATA_UI_BUTTON) {
    PayloadButton *btn = (PayloadButton *)n->data_ptr;
    strncpy(btn->on_click_cmd, cmd, 127);
    printf("[TRINITY] Bound event to Node %d: '%s'\n", id, cmd);
  }
}

bool trinity_get_binding(NodeID id, int event_type, char *out_cmd) {
  (void)event_type;
  Node *n = get_node(id);
  if (!n)
    return false;

  if (n->data_type == DATA_UI_BUTTON) {
    PayloadButton *btn = (PayloadButton *)n->data_ptr;
    if (strlen(btn->on_click_cmd) > 0) {
      strncpy(out_cmd, btn->on_click_cmd, 127);
      return true;
    }
  }
  return false;
}

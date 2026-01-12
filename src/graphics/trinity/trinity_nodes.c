// #include "../../graphics/backend/opengl_es_backend.h"
#include "pufu/graphics.h"
#include "pufu/trinity.h" // For Enums
#include "trinity_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STB_IMAGE_IMPLEMENTATION defined elsewhere (assets.c)?
// We need stbi_load. If it is already linked via assets.o, we just need header.
// But we need declaration.
#include "../include/stb_image.h"

// --- Helper funcs ---
Node *get_node(NodeID id) {
  if (id <= 0 || id > node_count)
    return NULL;
  return &node_pool[id - 1];
}

NodeID trinity_create_node(const char *name, NodeType type) {
  if (node_count >= MAX_NODES) {
    printf("[TRINITY] Error: Max nodes reached.\n");
    return -1;
  }

  Node *n = &node_pool[node_count];
  n->id = node_count + 1;
  strncpy(n->name, name, MAX_NAME_LEN - 1);
  n->type = type;
  n->flags = NODE_FLAG_VISIBLE | NODE_FLAG_PERSIST; // Default flags

  // Default transforms
  n->scale.x = 1.0f;
  n->scale.y = 1.0f;
  n->scale.z = 1.0f;

  // Asignar Payload según tipo (Polimorfismo manual)
  n->data_ptr = NULL;
  n->data_size = 0;

  switch (type) {
  case NODE_MESH:
  case NODE_SKYBOX:
    n->data_type = DATA_MESH_BUFFER;
    n->data_ptr = malloc(sizeof(PayloadMeshBuffer));
    memset(n->data_ptr, 0, sizeof(PayloadMeshBuffer));
    break;
  case NODE_CAMERA:
    n->data_type = DATA_CAMERA;
    n->data_ptr = malloc(sizeof(PayloadCamera));
    memset(n->data_ptr, 0, sizeof(PayloadCamera));
    break;
  case NODE_LIGHT:
    n->data_type = DATA_LIGHT;
    n->data_ptr = malloc(sizeof(PayloadLight));
    memset(n->data_ptr, 0, sizeof(PayloadLight));
    break;
  case NODE_PROCEDURAL:
    n->data_type = DATA_MESH_PROCEDURAL;
    n->data_ptr = malloc(sizeof(PayloadProcedural));
    // Default Init
    ((PayloadProcedural *)n->data_ptr)->generator = NULL;
    ((PayloadProcedural *)n->data_ptr)->frequency = 1.0f;
    break;
  case NODE_WINDOW:
    n->data_type = DATA_WINDOW;
    n->data_ptr = malloc(sizeof(PayloadWindow));
    // Resolution Defaults
    ((PayloadWindow *)n->data_ptr)->width = 800;
    ((PayloadWindow *)n->data_ptr)->height = 600;
    break;
  case NODE_GEAR:
    n->data_type = DATA_NONE; // Gear is pure logic, no payload for now
    n->data_ptr = NULL;
    break;
  case NODE_UI_BUTTON:
    n->data_type = DATA_UI_BUTTON;
    n->data_ptr = malloc(sizeof(PayloadButton));
    memset(n->data_ptr, 0, sizeof(PayloadButton));
    // Defaults
    ((PayloadButton *)n->data_ptr)->width = 100.0f;
    ((PayloadButton *)n->data_ptr)->height = 40.0f;
    ((PayloadButton *)n->data_ptr)->color_on =
        (vec3){0.3f, 0.3f, 0.3f}; // Dark Grey (Pressed)
    ((PayloadButton *)n->data_ptr)->color_off =
        (vec3){0.8f, 0.8f, 0.8f}; // Light Grey (Released)
    break;
  case NODE_UI_IMAGE:
    n->data_type = DATA_UI_IMAGE;
    n->data_ptr = malloc(sizeof(PayloadImage));
    memset(n->data_ptr, 0, sizeof(PayloadImage));
    break;
  case NODE_UI_WINDOW:
    n->data_type = DATA_FRAME;
    n->data_ptr = malloc(sizeof(PayloadFrame));
    memset(n->data_ptr, 0, sizeof(PayloadFrame));
    // Defaults
    ((PayloadFrame *)n->data_ptr)->x = 50.0f;
    ((PayloadFrame *)n->data_ptr)->y = 50.0f;
    ((PayloadFrame *)n->data_ptr)->width = 400.0f;
    ((PayloadFrame *)n->data_ptr)->height = 300.0f;
    // Fractal Defaults
    ((PayloadFrame *)n->data_ptr)->movable = true;
    ((PayloadFrame *)n->data_ptr)->header_id = -1;
    ((PayloadFrame *)n->data_ptr)->body_id = -1;
    ((PayloadFrame *)n->data_ptr)->bg_color = (vec3){0.2f, 0.2f, 0.2f};
    strcpy(((PayloadFrame *)n->data_ptr)->title, "Frame");
    break;
  default:
    n->data_type = DATA_NONE;
    break;
  }

  printf("[TRINITY] Node Created: '%s' (ID: %d, Type: %d)\n", name, n->id,
         n->data_type);

  node_count++;
  return n->id;
}

NodeID trinity_get_node_id(const char *name) {
  for (int i = 0; i < node_count; i++) {
    if (strcmp(node_pool[i].name, name) == 0) {
      return node_pool[i].id;
    }
  }
  return -1;
}

void trinity_set_vec3(NodeID id, const char *prop_name, float x, float y,
                      float z) {
  Node *n = get_node(id);
  if (!n)
    return;

  if (strcmp(prop_name, "pos") == 0) {
    n->position.x = x;
    n->position.y = y;
    n->position.z = z;
    printf("[TRINITY] Set %s.pos = (%.1f, %.1f, %.1f)\n", n->name, x, y, z);
  } else if (strcmp(prop_name, "view") == 0 && n->data_type == DATA_CAMERA) {
    PayloadCamera *cam = (PayloadCamera *)n->data_ptr;
    cam->target.x = x;
    cam->target.y = y;
    cam->target.z = z;
  }

  if (n->data_type == DATA_UI_BUTTON) {
    PayloadButton *btn = (PayloadButton *)n->data_ptr;
    if (strcmp(prop_name, "color_on") == 0) {
      btn->color_on.x = x;
      btn->color_on.y = y;
      btn->color_on.z = z;
    } else if (strcmp(prop_name, "color_off") == 0) {
      btn->color_off.x = x;
      btn->color_off.y = y;
      btn->color_off.z = z;
    }
  }
}

void trinity_set_vec4(NodeID id, const char *prop_name, float r, float g,
                      float b, float a) {
  Node *n = get_node(id);
  if (!n)
    return;

  if (strcmp(prop_name, "rect") == 0) {
    if (n->data_type == DATA_UI_BUTTON) {
      PayloadButton *btn = (PayloadButton *)n->data_ptr;
      btn->x = r;
      btn->y = g;
      btn->width = b;
      btn->height = a;
      printf("[TRINITY] Set Button Rect: %s (%.1f, %.1f, %.1f, %.1f)\n",
             n->name, r, g, b, a);
    } else if (n->data_type == DATA_UI_IMAGE) {
      PayloadImage *img = (PayloadImage *)n->data_ptr;
      img->x = r;
      img->y = g;
      img->width = b;
      img->height = a;
      printf("[TRINITY] Set Image Rect: %s (%.1f, %.1f, %.1f, %.1f)\n", n->name,
             r, g, b, a);
    } else if (n->data_type == DATA_FRAME) {
      PayloadFrame *win = (PayloadFrame *)n->data_ptr;
      win->x = r;
      win->y = g;
      win->width = b;
      win->height = a;
      printf("[TRINITY] Set Frame Rect: %s (%.1f, %.1f, %.1f, %.1f)\n", n->name,
             r, g, b, a);
    }
  }
}

void trinity_set_string(NodeID id, const char *prop_name, const char *val) {
  Node *n = get_node(id);
  if (!n)
    return;

  if (strcmp(prop_name, "color") == 0) {
    printf("[TRINITY] Set %s.color = %s\n", n->name, val);
  } else if (strcmp(prop_name, "label") == 0 &&
             n->data_type == DATA_UI_BUTTON) {
    PayloadButton *btn = (PayloadButton *)n->data_ptr;
    strncpy(btn->label, val, 63);
    btn->text_dirty = true;
    printf("[TRINITY] Set Button Label: %s -> '%s' (Deferred Bake)\n", n->name,
           val);
  } else if (strcmp(prop_name, "icon") == 0 && n->data_type == DATA_UI_BUTTON) {
    PayloadButton *btn = (PayloadButton *)n->data_ptr;
    // Load Icon Immediately
    int w = 0, h = 0; // Initialize for SVG loader safety
    int ch;

    // Check extension
    const char *ext = strrchr(val, '.');
    bool is_svg = (ext && strcmp(ext, ".svg") == 0);

    unsigned char *data = NULL;
    int tex_id = 0;

    if (is_svg) {
      if (renderer_active) {
        tex_id = trinity_load_svg(val, &w, &h);
      }
      // If not active, we can't really load SVG yet as it requires
      // rasterizer/context? Actually nanosvg allows rasterization without GL
      // context. But uploading texture requires GL. So we can rasterize to
      // pixels here? trinity_load_svg uploads internally. Let's rely on it
      // returning 0 if not active? Wait, see trinity_load_svg implementation:
      // if (renderer_active) upload. It returns 0 if not active BUT it
      // rasterizes. We probably want to defer if not active.
    } else {
      data = stbi_load(val, &w, &h, &ch, 4); // Force RGBA
      if (data && renderer_active) {
        tex_id = trinity_renderer_create_texture(w, h, data);
      }
    }

    if (renderer_active && tex_id > 0) {
      btn->icon_texture_id = tex_id;
      btn->icon_w = w;
      btn->icon_h = h;
      printf("[TRINITY] Loaded Icon '%s' (%dx%d)\n", val, w, h);
    }

    if (data)
      stbi_image_free(data);

  } else if (strcmp(prop_name, "src") == 0 && n->data_type == DATA_UI_IMAGE) {
    PayloadImage *img = (PayloadImage *)n->data_ptr;
    strncpy(img->path, val, 127);

    if (renderer_active) {
      int w = 0, h = 0, ch;
      int tex_id = 0;

      const char *ext = strrchr(val, '.');
      if (ext && strcmp(ext, ".svg") == 0) {
        tex_id = trinity_load_svg(val, &w, &h);
      } else {
        unsigned char *pixels = stbi_load(val, &w, &h, &ch, 4);
        if (pixels) {
          tex_id = trinity_renderer_create_texture(w, h, pixels);
          stbi_image_free(pixels);
        }
      }

      if (tex_id > 0) {
        img->texture_id = tex_id;
        img->width = w; // Auto-resize
        img->height = h;
        printf("[TRINITY] Loaded Image '%s' (%dx%d) for Node %s\n", val, w, h,
               n->name);
      }
    } else {
      // Deferred loading handled in trinity_prepare
      printf("[TRINITY] Image '%s' set for deferred load.\n", val);
    }
  } else if (strcmp(prop_name, "title") == 0 && n->data_type == DATA_FRAME) {
    // Just set title field, no usage yet
    PayloadFrame *f = (PayloadFrame *)n->data_ptr;
    strncpy(f->title, val, 63);
  } else if (strcmp(prop_name, "movable") == 0 && n->data_type == DATA_FRAME) {
    PayloadFrame *f = (PayloadFrame *)n->data_ptr;
    if (strcmp(val, "false") == 0 || strcmp(val, "0") == 0) {
      f->movable = false;
      printf("[TRINITY] Frame '%s' set to Immutable (Desktop Mode).\n",
             n->name);
    } else {
      f->movable = true;
    }
  }
}

void trinity_link_nodes(NodeID parent, NodeID child) {
  Node *p = get_node(parent);
  Node *c = get_node(child);
  if (!p || !c)
    return;
  if (p->children_count < MAX_CHILDREN) {
    p->children[p->children_count++] = child;
    c->parent_id = parent;
    printf("[TRINITY] Linked '%s' -> child '%s'\n", p->name, c->name);
  }
}

void trinity_node_add_list(NodeID space, NodeID *children, int count) {
  for (int i = 0; i < count; i++) {
    trinity_link_nodes(space, children[i]);
  }
}

void trinity_bind_control(NodeID gear, NodeID controlled_node) {
  printf("[TRINITY] Gear '%d' now controls Node '%d'\n", gear, controlled_node);
}

void trinity_set_procedural_generator(NodeID id, ProceduralFunc func) {
  Node *n = get_node(id);
  if (!n)
    return;
  if (n->data_type == DATA_MESH_PROCEDURAL && n->data_ptr) {
    ((PayloadProcedural *)n->data_ptr)->generator = func;
    printf("[TRINITY] Set procedural generator for node %s\n", n->name);
  }
}

void trinity_get_vec4(NodeID id, const char *prop_name, float *out_vec) {
  Node *n = get_node(id);
  if (!n || !out_vec)
    return;
  if (strcmp(prop_name, "rect") == 0) {
    float x = 0, y = 0, w = 0, h = 0;
    if (n->data_type == DATA_FRAME) {
      PayloadFrame *win = (PayloadFrame *)n->data_ptr;
      x = win->x;
      y = win->y;
      w = win->width;
      h = win->height;
    } else if (n->data_type == DATA_UI_BUTTON) {
      PayloadButton *btn = (PayloadButton *)n->data_ptr;
      x = btn->x;
      y = btn->y;
      w = btn->width;
      h = btn->height;
    } else if (n->data_type == DATA_UI_IMAGE) {
      PayloadImage *img = (PayloadImage *)n->data_ptr;
      x = img->x;
      y = img->y;
      w = img->width;
      h = img->height;
    }
    out_vec[0] = x;
    out_vec[1] = y;
    out_vec[2] = w;
    out_vec[3] = h;
  }
}

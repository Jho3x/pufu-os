#include "pufu/loader.h"
#include "pufu/graphics.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Utils
static char *trim_whitespace(char *str) {
  char *end;
  while (isspace((unsigned char)*str))
    str++;
  if (*str == 0)
    return str;
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;
  end[1] = '\0';
  return str;
}

// Extract key:value from string "key:value" or "key:\"value\""
// Returns pointer to value, modifies input string to null terminate key
static char *parse_kv(char *arg, char **key_out) {
  char *colon = strchr(arg, ':');
  if (!colon)
    return NULL;

  *colon = '\0';
  *key_out = trim_whitespace(arg);

  char *val = colon + 1;
  val = trim_whitespace(val);

  // Remove quotes if present
  if (val[0] == '"') {
    val++;
    char *end_quote = strrchr(val, '"');
    if (end_quote)
      *end_quote = '\0';
  }
  return val;
}

// Helper to strip quotes
static void strip_quotes(char *str) {
  int len = strlen(str);
  if (len < 2)
    return;
  if (str[0] == '"' && str[len - 1] == '"') {
    memmove(str, str + 1, len - 2);
    str[len - 2] = '\0';
  }
}

// Helper to parse vec3 (x, y, z)
// Parse tuple "(x,y,z)" to Vec3
static void parse_vec3(const char *str, Vec3 *out) {
  // Simple sscanf, assumes clean input for now
  // Create copy to handle parentheses removal if needed, or just sscanf skip
  // Format: (1.0, 2.0, 3.0) or 1,2,3
  const char *p = str;
  if (*p == '(')
    p++;
  sscanf(p, "%f, %f, %f", &out->x, &out->y, &out->z);
}

// Specific Command Handlers
static void handle_new(Scene *scene, char *content) {
  // new(class:"alias", class2:"alias2", ...)
  // content is inside parens: sphere:"ball", light:"luz"

  char *ptr = content;
  char *arg_start = ptr;

  while (*ptr) {
    // Find end of current argument (comma or end of string)
    if (*ptr == ',') {
      *ptr = '\0'; // Split

      char *key;
      char *alias = parse_kv(arg_start, &key);
      if (alias && key) {
        // ... Creation Logic (Duplicate 1) ...
        PufuEntity *e = scene_create_entity(scene, alias);
        if (strcmp(key, "sphere") == 0)
          entity_add_component(e, COMP_MODEL, 2); // ID 2 = Sphere Primitive
        else if (strcmp(key, "camera") == 0 || strcmp(key, "cam") == 0)
          entity_add_component(e, COMP_CAMERA,
                               0); // Param1 will be set by 'type' later
        else if (strcmp(key, "light") == 0)
          entity_add_component(e, COMP_LIGHT, 0);
        else if (strcmp(key, "fire") == 0)
          entity_add_component(e, COMP_EMITTER, 1); // ID 1 = Fire Preset
        else if (strcmp(key, "skybox") == 0)
          entity_add_component(e, COMP_SKYBOX, 0);
        // Ignore "Pufu", "scene", "window", "gear" for now as they are system
        // objects in Python proto Or map them to a dummy entity for "set"
        // commands
        else if (strcmp(key, "object") == 0)
          entity_add_component(e, COMP_MODEL, 1);
        else if (strcmp(key, "text") == 0)
          entity_add_component(e, COMP_TEXT, 0);
        else if (strcmp(key, "scene") == 0)
          scene_create_entity(scene, alias); // space container
      }

      arg_start = ptr + 1; // Next arg
    }
    ptr++;
  }

  // Process last arg
  char *key;
  char *alias = parse_kv(arg_start, &key);
  if (alias && key) {
    // ... Creation Logic (Duplicate 2) ...
    // Ideally extract this to helper 'create_by_type'
    PufuEntity *e = scene_create_entity(scene, alias);
    if (strcmp(key, "sphere") == 0)
      entity_add_component(e, COMP_MODEL, 2); // ID 2 = Sphere Primitive
    else if (strcmp(key, "camera") == 0 || strcmp(key, "cam") == 0)
      entity_add_component(e, COMP_CAMERA, 0);
    else if (strcmp(key, "light") == 0)
      entity_add_component(e, COMP_LIGHT, 0);
    else if (strcmp(key, "fire") == 0)
      entity_add_component(e, COMP_EMITTER, 1); // ID 1 = Fire Preset
    else if (strcmp(key, "skybox") == 0)
      entity_add_component(e, COMP_SKYBOX, 0);
    else if (strcmp(key, "object") == 0)
      entity_add_component(e, COMP_MODEL, 1); // Default to Cube/Empty
    else if (strcmp(key, "scene") == 0)
      scene_create_entity(scene, alias);
  }
}

static void process_set_kv(Scene *scene, PufuEntity *e, char *kv_str) {
  char *key;
  char *val = parse_kv(kv_str, &key);

  if (key && val) {
    if (strcmp(key, "pos") == 0) {
      Vec3 v;
      parse_vec3(val, &v);
      entity_set_pos(e, v.x, v.y, v.z);
    } else if (strcmp(key, "scene") == 0) {
      // scene_set_active_camera(scene, val); // Disabled: 'espacio' is not a
      // camera
    } else if (strcmp(key, "camera") == 0) {
      scene_set_active_camera(scene, val);
    } else if (strcmp(key, "texture") == 0) {
      strip_quotes(val); // FIX: Remove quotes from filename
      printf("Trinity Loader: Loading texture '%s' for entity '%s'\n", val,
             e->name);
      int tex_id = pufu_graphics_load_texture(val, NULL, NULL);

      int assigned = 0;
      for (int i = 0; i < e->component_count; i++) {
        if (e->components[i].type == COMP_MODEL ||
            e->components[i].type == COMP_SKYBOX ||
            e->components[i].type == COMP_EMITTER) {
          e->components[i].param1.y = (float)tex_id; // Store Texture ID in Y
          printf("Trinity Loader: Assigned Texture ID %d to PufuEntity '%s' "
                 "(CompType %d)\n",
                 tex_id, e->name, e->components[i].type);
          assigned = 1;
        }
      }
      if (!assigned) {
        printf("Trinity Warning: Texture loaded but no compatible component "
               "found for entity '%s'\n",
               e->name);
      }

    } else if (strcmp(key, "behavior") == 0) {
      if (strcmp(val, "rotate_y") == 0) {
        // ID 1 for Rotate Y behavior
        entity_add_component(e, COMP_SCRIPT, 1);
      }
    } else if (strcmp(key, "type") == 0) {
      // Don't strip quotes for types like "skybox" unless they have them?
      // Pufu parser might keep them. Let's strip just in case.
      strip_quotes(val);

      if (strcmp(val, "skybox") == 0 || strcmp(val, "skymesh") == 0) {
        entity_add_component(e, COMP_SKYBOX, 0);
      } else if (strcmp(val, "sphere") == 0) {
        entity_add_component(e, COMP_MODEL, 2); // Sphere
      } else if (strcmp(val, "camera") == 0) {
        entity_add_component(e, COMP_CAMERA, 0);
      } else if (strcmp(val, "light") == 0) {
        entity_add_component(e, COMP_LIGHT, 0);
      } else if (strcmp(val, "fire") == 0) {
        // ID 1 for Fire Emitter
        entity_add_component(e, COMP_EMITTER, 1);
        // Auto-load default fire texture
        int tex_id = pufu_graphics_load_texture("data/fire1.png", NULL, NULL);
        if (e->component_count > 0) {
          e->components[e->component_count - 1].param1.y = (float)tex_id;
        }
      } else if (strcmp(val, "follow") == 0) {
        // ID 2 for Follow/Billboard
        entity_add_component(e, COMP_SCRIPT, 2);
        printf("Trinity Loader: Added 'follow' behavior to '%s'\n", e->name);
      } else if (strcmp(val, "look_from") == 0) {
        int has_cam = 0;
        for (int i = 0; i < e->component_count; i++)
          if (e->components[i].type == COMP_CAMERA)
            has_cam = 1;
        if (!has_cam)
          entity_add_component(e, COMP_CAMERA, 0);
      } else if (strcmp(val, "look_at") == 0) {
        int has_cam = 0;
        for (int i = 0; i < e->component_count; i++)
          if (e->components[i].type == COMP_CAMERA)
            has_cam = 1;
        if (!has_cam)
          entity_add_component(e, COMP_CAMERA, 0);
        // Mark as look_at (mode 1)
        for (int i = 0; i < e->component_count; i++)
          if (e->components[i].type == COMP_CAMERA)
            e->components[i].param1.x = 1.0f;
      }
    } else if (strcmp(key, "string") == 0) {
      // Handle Text Content
      // Find COMP_TEXT
      int found = 0;
      for (int i = 0; i < e->component_count; i++) {
        if (e->components[i].type == COMP_TEXT) {
          e->components[i].data = strdup(val);
          printf("Trinity Loader: Set text content for '%s': %s\n", e->name,
                 val);
          found = 1;
          break;
        }
      }
      if (!found) {
        // Implicitly add text component if setting string?
        entity_add_component(e, COMP_TEXT, 0);
        if (e->component_count > 0)
          e->components[e->component_count - 1].data = strdup(val);
      }
    }
  }
}

static void handle_set(Scene *scene, PufuEntity *e, char *content) {
  // Split by comma respecting parens () and quotes ""
  char *ptr = content;
  char *arg_start = content;
  int nesting = 0;
  int in_quote = 0;

  while (*ptr) {
    if (*ptr == '"') {
      in_quote = !in_quote;
    } else if (!in_quote) {
      if (*ptr == '(') {
        nesting++;
      } else if (*ptr == ')') {
        if (nesting > 0)
          nesting--;
      } else if (*ptr == ',' && nesting == 0) {
        *ptr = '\0'; // Terminate current arg
        process_set_kv(scene, e, arg_start);
        arg_start = ptr + 1; // Start next arg
      }
    }
    ptr++;
  }
  // Process last arg
  if (*arg_start) {
    process_set_kv(scene, e, arg_start);
  }
}

static void handle_add(Scene *scene, Entity *e, char *content) {
  (void)scene;
  (void)e;
  (void)content;
  // espacio.add(lz, cam1, ...)
  // In our semantic, 'add' to scene is implied by creation.
  // But maybe parenting?
  // In demo: espacio.add(luz, cam1, ...) seems to be just enabling/grouping.
  // We already added them to scene on 'new'. So we might ignore or verify
  // logic. For now no-op unless parent logic needed.
}

int loader_parse_line(Scene *scene, char *line) {
  line = trim_whitespace(line);
  if (strlen(line) == 0 || line[0] == '#')
    return 0;

  // 1. Check for "new"
  if (strncmp(line, "new", 3) == 0) {
    char *paren = strchr(line, '(');
    if (paren) {
      char *end = strrchr(paren, ')');
      if (end)
        *end = '\0';
      handle_new(scene, paren + 1);
    }
    return 0;
  }

  // 2. Subject.Action
  char *dot = strchr(line, '.');
  if (dot) {
    *dot = '\0';
    char *subject_name = trim_whitespace(line);
    char *rest = dot + 1;

    char *paren = strchr(rest, '(');
    if (paren) {
      *paren = '\0';
      char *action = trim_whitespace(rest);
      char *content = paren + 1;
      char *end = strrchr(content, ')');
      if (end)
        *end = '\0';

      // Resolve Subject
      Entity *subject = scene_find_entity(scene, subject_name);
      // Special subjects: "motor", "espacio" (treated as scene root)
      if (strcmp(subject_name, "espacio") == 0) {
        // Scene level actions
        if (strcmp(action, "set") == 0)
          handle_set(scene, NULL, content);
      } else if (subject) {
        if (strcmp(action, "set") == 0)
          handle_set(scene, subject, content);
        if (strcmp(action, "add") == 0)
          handle_add(scene, subject, content);
      }
    }
  }

  return 0;
}

int loader_load_file(Scene *scene, const char *filepath) {
  FILE *f = fopen(filepath, "r");
  if (!f) {
    printf("Parser: Error opening file %s\n", filepath);
    return -1;
  }

  char line[256];
  int inside_block = 0;

  while (fgets(line, sizeof(line), f)) {
    // Tag Check
    if (strstr(line, "_meow::init")) {
      inside_block = 1;
      continue;
    }
    if (strstr(line, "_meow::stop")) { // Using new 'stop' tag
      inside_block = 0;
      break;
    }

    // Auto-detect compatibility (if no tags found yet, maybe legacy or pure
    // file?) The spec says use tags. We enforce strictness for now, OR we
    // assume if we are in this function we want to parse everything if no tags
    // present? Let's implement strict block logic first.

    if (inside_block) {
      loader_parse_line(scene, line);
    }
  }

  fclose(f);
  return 0;
}

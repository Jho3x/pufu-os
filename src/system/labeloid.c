#include "pufu/labeloid.h"
#include "pufu/engine.h"
#include "pufu/loader.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple trims borrowed from loader.c
// Simple trims borrowed from loader.c (Unused currently)
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

static char *parse_kv(char *arg, char **key_out) {
  char *colon = strchr(arg, ':');
  if (!colon)
    return NULL;
  *colon = '\0';
  // *key_out = trim_whitespace(arg);
  *key_out = arg; // trim_whitespace disabled
  char *val = colon + 1;
  // val = trim_whitespace(val);
  if (val[0] == '"') {
    val++;
    char *end_quote = strrchr(val, '"');
    if (end_quote)
      *end_quote = '\0';
  }
  return val;
}

void strip_quotes(char *str) {
  int len = strlen(str);
  if (len < 2)
    return;
  if (str[0] == '"' && str[len - 1] == '"') {
    memmove(str, str + 1, len - 2);
    str[len - 2] = '\0';
  }
}

// Internal: Handle 'new(...)' command by calling Trinity Engine
// Internal: Handle 'new(...)' command by calling Trinity Engine
#if 0
static void labeloid_handle_new(char *content) {
  // Supports both:
  // 1. new(class:"alias", class2:"alias2")
  // 2. class="alias" , class2="alias2"

  char *ptr = content;
  char *arg_start = ptr;

  while (*ptr) {
    if (*ptr == ',') {
      *ptr = '\0';
      char *key;
      // parse_kv expects 'key: val'. We need to handle 'key = val' too.
      // Let's patch parse_kv or do simple replacement here?
      // parse_kv uses colon. Let's pre-process the segment?
      // No, let's just make a smarter parse_kv or patch it here.

      // Temporary: Replace first '=' with ':' in the segment
      char *eq = strchr(arg_start, '=');
      if (eq)
        *eq = ':';

      char *alias = parse_kv(arg_start, &key);
      if (alias && key) {
        // Map Meow Class to Trinity Type
        int type = NODE_ROOT;
        if (strcmp(key, "gear") == 0)
          type = NODE_GEAR;
        else if (strcmp(key, "window") == 0)
          type = NODE_WINDOW;
        else if (strcmp(key, "sphere") == 0)
          type = NODE_MESH;
        else if (strcmp(key, "light") == 0)
          type = NODE_LIGHT;
        else if (strcmp(key, "camera") == 0)
          type = NODE_CAMERA;
        else if (strcmp(key, "space") == 0)
          type = NODE_SPACE;
        else if (strcmp(key, "skybox") == 0)
          type = NODE_SKYBOX;
        else if (strcmp(key, "emitter") == 0)
          type = NODE_EMITTER;

        printf("[LABELOID] Spawning Node: %s (Type %d)\n", alias, type);
        trinity_create_node(alias, type);
      }
      arg_start = ptr + 1;
    }
    ptr++;
  }
  // Last arg
  char *eq = strchr(arg_start, '=');
  if (eq)
    *eq = ':';

  char *key;
  char *alias = parse_kv(arg_start, &key);
  if (alias && key) {
    int type = NODE_ROOT;
    if (strcmp(key, "gear") == 0)
      type = NODE_GEAR;
    else if (strcmp(key, "window") == 0)
      type = NODE_WINDOW;
    else if (strcmp(key, "sphere") == 0)
      type = NODE_MESH;
    else if (strcmp(key, "light") == 0)
      type = NODE_LIGHT;
    else if (strcmp(key, "camera") == 0)
      type = NODE_CAMERA;
    else if (strcmp(key, "space") == 0)
      type = NODE_SPACE;
    else if (strcmp(key, "skybox") == 0)
      type = NODE_SKYBOX;
    else if (strcmp(key, "emitter") == 0)
      type = NODE_EMITTER;
    printf("[LABELOID] Spawning Node: %s (Type %d)\n", alias, type);
    trinity_create_node(alias, type);
  }
}
#endif

// Internal: Handle 'alias.set(...)'
#if 0
static void labeloid_handle_set(char *subject, char *content) {
  // For now, simpler parsing than full recursive loader.c
  // We assume simple key:value pairs for demo
  char *ptr = content;
  char *arg_start = ptr;

  // We need to find the node ID first?
  // Trinity doesn't expose find_by_name easily yet?
  // Actually trinity_create_node returns ID but we don't store it here easily.
  // We rely on the fact that we can implement `trinity_set_property(name, key,
  // value)` OR just use logic to update payload. For Setup Phase, just logging.

  // NOTE: This needs connection to `trinity_implementation.c` to be useful.
  // I will add `trinity_update_node(name, key, value)` later.
  printf("[LABELOID] Setting Property on '%s': %s\n", subject, content);

  // Very Basic Parser for single properties
  // e.g. pos:(0,0,0) type:"skybox"
  // This requires implementing the specific setters using Trinity API.
  // For now, we delegate this to Phase 4 (Full Parser Integration).
}
#endif

int labeloid_init(void) {
  printf("[LABELOID] Immune System Initialized.\n");
  return 0;
}

int labeloid_cast(const char *filepath, CastingMode mode) {
  printf("[LABELOID] Casting %s (Mode %d)...\n", filepath, mode);
  // In future: Full transpilation logic
  // For now: Just pass through to load_scene if it's Meow
  if (mode == CAST_MEOW_TO_PAW) {
    return labeloid_load_scene(filepath);
  }
  return 0;
}

int labeloid_load_scene(const char *filepath) {
  FILE *f = fopen(filepath, "r");
  if (!f)
    return -1;

  char line[256];
  int inside_pufu_init = 0;
  int inside_meow = 0;

  while (fgets(line, sizeof(line), f)) {
    // 1. Check Global Pufu Block
    if (strstr(line, "_pufu::init")) {
      inside_pufu_init = 1;
      continue;
    }
    if (strstr(line, "_pufu::stop")) {
      inside_pufu_init = 0;
      inside_meow = 0; // Force close sub-blocks
      continue;
    }

    // 2. Parsers only care about structure within _pufu::init ??
    // Actually, user said: "_pufu::init ... _pufu::meow ... _pufu::stop"
    // So we should only look for meow IF inside init?
    // Let's be safe and check for _pufu::meow generally or inside init.

    if (inside_pufu_init && strstr(line, "_pufu::meow")) {
      inside_meow = 1;
      continue;
    }
    // We can assume _pufu::meow runs until the next _pufu::tag or _pufu::stop?
    // User didn't specify closing tag for sub-languages explicitly aside from
    // `stop` closing `init`. But logically, if we hit `_pufu::paw` or
    // `_pufu::stop`, meow ends.
    if (strstr(line, "_pufu::paw") || strstr(line, "_pufu::claw") ||
        strstr(line, "_pufu::stop")) {
      inside_meow = 0;
    }

    if (inside_meow) {
      char *clean = trim_whitespace(line);
      // Check for Assignment: key = "value" (Implicit Declaration)
      if (strchr(clean, '=') && !strstr(clean, "new") &&
          !strstr(clean, ".set")) {
        char *ptr = clean;
        while (*ptr) {
          char *comma = strchr(ptr, ',');
          if (comma)
            *comma = 0;

          char *eq = strchr(ptr, '=');
          if (eq) {
            char *val = eq + 1;
            val = trim_whitespace(val);
            if (val[0] == '"') { // Strip quotes
              val++;
              char *endq = strrchr(val, '"');
              if (endq)
                *endq = 0;
            }
            if (strlen(val) > 0) {
              printf("[LABELOID] Implicit Declaration: Creating Entity '%s'\n",
                     val);
              scene_create_entity(engine_get_scene(), val);
            }
          }

          if (!comma)
            break;
          ptr = comma + 1;
        }
      } else {
        // Delegate other commands (new, set, etc.) to Loader
        loader_parse_line(engine_get_scene(), line);
      }
    }
  }

  fclose(f);
  return 0;
}

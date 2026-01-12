#include "meow_parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Symbol Table (Variables) ---

#define MAX_VARS 64

typedef struct {
  char *name;
  char *value_str; // For aliases like "ventana"
  NodeID node_id;  // For resolved objects
} MeowVar;

static MeowVar g_vars[MAX_VARS];
static int g_var_count = 0;

static void clear_vars() {
  for (int i = 0; i < g_var_count; i++) {
    free(g_vars[i].name);
    if (g_vars[i].value_str)
      free(g_vars[i].value_str);
  }
  g_var_count = 0;
}

static MeowVar *find_var(const char *name) {
  for (int i = 0; i < g_var_count; i++) {
    if (strcmp(g_vars[i].name, name) == 0) {
      return &g_vars[i];
    }
  }
  return NULL;
}

static void set_var(const char *name, const char *val_str, NodeID id) {
  MeowVar *v = find_var(name);
  if (!v) {
    if (g_var_count >= MAX_VARS)
      return; // Overflow
    v = &g_vars[g_var_count++];
    v->name = strdup(name);
  } else {
    if (v->value_str)
      free(v->value_str);
  }
  v->value_str = val_str ? strdup(val_str) : NULL;
  v->node_id = id;
}

// --- Lexer / Helper ---

static char *read_file(const char *filename) {
  FILE *f = fopen(filename, "rb");
  if (!f)
    return NULL;
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *string = malloc(fsize + 1);
  fread(string, 1, fsize, f);
  fclose(f);
  string[fsize] = 0;
  return string;
}

typedef struct {
  char *src;
  int pos;
  int len;
} ParserState;

static void skip_space(ParserState *p) {
  while (p->pos < p->len &&
         (isspace(p->src[p->pos]) || p->src[p->pos] == ',')) {
    p->pos++;
  }
}

// Check for specific char
static int match_char(ParserState *p, char c) {
  skip_space(p);
  if (p->pos < p->len && p->src[p->pos] == c) {
    p->pos++;
    return 1;
  }
  return 0;
}

// Parse string literal "text"
static char *parse_string(ParserState *p) {
  skip_space(p);
  if (p->src[p->pos] == '"') {
    p->pos++;
    int start = p->pos;
    while (p->pos < p->len && p->src[p->pos] != '"') {
      p->pos++;
    }
    int len = p->pos - start;
    char *str = malloc(len + 1);
    strncpy(str, p->src + start, len);
    str[len] = 0;
    if (p->pos < p->len)
      p->pos++; // skip closing quote
    return str;
  }
  return NULL;
}

// Parse identifier (alphanumeric + usually starts with letter)
static char *parse_identifier(ParserState *p) {
  skip_space(p);
  int start = p->pos;
  if (!isalpha(p->src[p->pos]) && p->src[p->pos] != '_')
    return NULL;

  while (p->pos < p->len && (isalnum(p->src[p->pos]) || p->src[p->pos] == '_'))
    p->pos++;

  int len = p->pos - start;
  if (len == 0)
    return NULL;
  char *str = malloc(len + 1);
  strncpy(str, p->src + start, len);
  str[len] = 0;
  return str;
}

// Parse value (String or Float/Int as string, or Tuple)
// For simplicity, we mostly treat everything as strings in this version
// But we need to handle "480*320" or "(0,0,0)"
static char *parse_value_token(ParserState *p) {
  skip_space(p);
  if (p->src[p->pos] == '"')
    return parse_string(p);

  // If tuple (x,y,z)
  if (p->src[p->pos] == '(') {
    int start = p->pos;
    while (p->pos < p->len && p->src[p->pos] != ')')
      p->pos++;
    if (p->pos < p->len)
      p->pos++; // skip )
    int len = p->pos - start;
    char *str = malloc(len + 1);
    strncpy(str, p->src + start, len);
    str[len] = 0;
    return str;
  }

  // Raw value (numbers/symbols) until comma or paren
  int start = p->pos;
  while (p->pos < p->len && p->src[p->pos] != ',' && p->src[p->pos] != ')' &&
         !isspace(p->src[p->pos]))
    p->pos++;

  int len = p->pos - start;
  char *str = malloc(len + 1);
  strncpy(str, p->src + start, len);
  str[len] = 0;
  return str;
}

// --- execution logic ---

// Helper to parse "x y w h" or "x,y,w,h" from string
static void parse_vec4_str(const char *str, float *x, float *y, float *w,
                           float *h) {
  // Handle (x, y, z) format or space separated
  // Simple sscanf
  // Try to skip parens
  const char *ptr = str;
  if (*ptr == '(')
    ptr++;
  sscanf(ptr, "%f %f %f %f", x, y, w, h);
  // Also try comma
  if (strstr(ptr, ",")) {
    sscanf(ptr, "%f,%f,%f,%f", x, y, w, h);
  }
}

static void apply_method(NodeID id, const char *arg_key, const char *arg_val) {
  if (strcmp(arg_key, "rect") == 0 || strcmp(arg_key, "rectangle") == 0) {
    float x = 0, y = 0, w = 0, h = 0;
    // Check for "480*320" format (Window)
    if (strstr(arg_val, "*")) {
      sscanf(arg_val, "%f*%f", &w, &h);
      // Window usually 0,0
      trinity_set_vec4(id, "rect", 0, 0, w, h);
    } else {
      parse_vec4_str(arg_val, &x, &y, &w, &h);
      trinity_set_vec4(id, "rect", x, y, w, h);
    }
  } else if (strcmp(arg_key, "color") == 0) {
    // Handle hex or name? For now assume vec3 "r g b" or name
    // If name, simple hack check
    float r = 0, g = 0, b = 0;
    if (strcmp(arg_val, "white") == 0) {
      r = 1;
      g = 1;
      b = 1;
    } else if (strcmp(arg_val, "black") == 0) {
      r = 0;
      g = 0;
      b = 0;
    } else if (strcmp(arg_val, "orange") == 0) {
      r = 1;
      g = 0.5;
      b = 0;
    } else if (strcmp(arg_val, "gray") == 0) {
      r = 0.2;
      g = 0.2;
      b = 0.2;
    } else {
      parse_vec4_str(arg_val, &r, &g, &b, &r); // reuse vars
    }
    trinity_set_vec3(id, "color", r, g, b);
  } else if (strcmp(arg_key, "label") == 0 || strcmp(arg_key, "string") == 0) {
    trinity_set_string(id, "label", arg_val);
  } else if (strcmp(arg_key, "onclick") == 0) {
    trinity_bind_event(id, 4, arg_val); // 4 = Click
  } else if (strcmp(arg_key, "pos") == 0) {
    float x = 0, y = 0, z = 0, w = 0;
    parse_vec4_str(arg_val, &x, &y, &z, &w);
    // trinity currently uses vec4 for rect, maybe "pos" maps to rect x/y?
    // or need a transform component?
    // For UI 2D, pos usually sets x/y of rect.
    // We'll map to rect assuming w/h are preserved?
    // Trinity doesn't have get_rect yet, so we blindly set x/y maybe?
    // Or we assume "pos" is strictly for 3D entities.
    // This parser is for UI primarily now.
    // Lets set x,y and keep w,h 0 if not known (This might be buggy for UI)
    // PROPER FIX: Load current rect?
    // For now, assume this is mainly for 3D props or we ignore "pos" for UI
    // unless it's full rect.
  }
}

static void execute_statement(ParserState *p) {
  // 1. Identifier
  char *ident = parse_identifier(p);
  if (!ident) {
    p->pos++; // skip char
    return;
  }

  skip_space(p);

  // 2. Assignment (=) or Dot (.)
  if (match_char(p, '=')) {
    // Assignment: var = "Value"
    char *val = parse_value_token(p); // Usually string "Window", "Button"
    if (val) {
      // Logic: Variable Declaration / Object Creation?
      // In trinity_demo: gear = "engranaje".
      // If "engranaje" is a reserved system object, we link to it.
      // If it's a new name like "TaskbarWindow", do we create it?
      // For UI, usually we Create explicitly or implicitly.
      // Let's assume implicit creation IF the value looks like a Type,
      // OR we just store the string name and create later??

      // Actually, look at taskbar_layout.pufu goal:
      // StartBtn.create(type: "Button")
      // So 'StartBtn = "MyButtonName"' just aliases the name.
      // The creation happens on .create() or .set()?

      // We'll Just Store Variable Mapping: ident -> val
      set_var(ident, val, -1);
      free(val);
    }
  } else if (match_char(p, '.')) {
    // Method Call: var.method(args)
    char *method = parse_identifier(p);
    if (method) {
      if (match_char(p, '(')) {
        // Parse Args: key: val, key: val
        while (p->pos < p->len && p->src[p->pos] != ')') {
          char *arg_key = parse_identifier(p);
          if (arg_key) {
            if (match_char(p, ':')) {
              char *arg_val = parse_value_token(p);
              if (arg_val) {
                // EXECUTE!
                MeowVar *v = find_var(ident);
                NodeID id = (v) ? v->node_id : -1;
                const char *obj_name = (v) ? v->value_str : ident;

                // Special Method: create
                if (strcmp(method, "create") == 0) {
                  // arg_key should be "type" ideally
                  NodeType type = NODE_UI_BUTTON;
                  if (strcmp(arg_val, "Window") == 0)
                    type = NODE_UI_WINDOW;
                  else if (strcmp(arg_val, "Image") == 0)
                    type = NODE_UI_IMAGE;

                  // Parent? "parent": "Name"
                  // trinity_create_node just makes it.
                  id = trinity_create_node(obj_name, type);
                  if (v)
                    v->node_id = id;
                } else if (strcmp(method, "add") == 0) {
                  // Space.add syntax?
                  // Ignore for UI currently
                } else {
                  // Setter
                  if (id != -1) {
                    apply_method(id, arg_key, arg_val);
                  }
                }
                free(arg_val);
              }
            }
            free(arg_key);
          }
          // Comma handling in skip_space
          skip_space(p);
        }
        match_char(p, ')'); // Consume closing paren
      }
      free(method);
    }
  }

  free(ident);
}

NodeID trinity_load_meow(const char *filename) {
  clear_vars();
  char *src = read_file(filename);
  if (!src)
    return -1;

  ParserState p = {src, 0, strlen(src)};

  // Check for _pufu::meow tag or just start?
  // We'll just parse statements
  while (p.pos < p.len) {
    skip_space(&p);
    // detect strict end
    if (p.pos >= p.len)
      break;

    // Check for comments #
    if (p.src[p.pos] == '#') {
      while (p.pos < p.len && p.src[p.pos] != '\n')
        p.pos++;
      continue;
    }

    execute_statement(&p);
  }

  free(src);
  // Return the last parsed node ID to the caller
  return (g_vars[g_var_count - 1].node_id > 0) ? g_vars[g_var_count - 1].node_id
                                               : 0;
}

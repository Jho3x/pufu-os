#ifndef TRINITY_INTERNAL_H
#define TRINITY_INTERNAL_H

#include "../include/stb/stb_truetype.h"
#include "trinity_types.h"
#include <stdbool.h>

// --- Global Constants ---
#define EVENT_QUEUE_SIZE 64
#define TRINITY_EXIT_OK 0
#define TRINITY_EXIT_BG 1

// --- Shared State (Defined in trinity_core.c) ---
extern Node node_pool[MAX_NODES];
extern int node_count;
extern bool renderer_active;

// --- Font State (Defined in trinity_render.c or core?) ---
// Let's define in trinity_render.c as it uses them most, but init is in core?
// Actually init is in trinity_init (Core).
extern stbtt_fontinfo g_font_info;
extern unsigned char *g_font_buffer;
extern bool g_font_loaded;

// --- Event State (Defined in trinity_events.c) ---
extern TrinityEvent g_event_queue[EVENT_QUEUE_SIZE];
extern int g_event_head;
extern int g_event_tail;
extern int g_event_count;

// --- Function Prototypes (Cross-Module) ---
Node *get_node(NodeID id);
void bake_label(PayloadButton *btn);
void trinity_render_frame(void);       // Main render routine
void trinity_prepare(void);            // Boot/Init visuals
void trinity_update_interaction(void); // Input logic
void trinity_queue_event(int type, int x, int y, NodeID target);
int trinity_load_svg(const char *filename, int *out_w, int *out_h);

#endif // TRINITY_INTERNAL_H

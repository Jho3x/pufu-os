#ifndef PUFU_LABELOID_H
#define PUFU_LABELOID_H

#include "pufu/trinity.h" // Access to Trinity Engine calls

// Casting Modes
typedef enum {
  CAST_MEOW_TO_PAW,
  CAST_CLAW_TO_PAW,
  CAST_PAW_TO_CLAW
} CastingMode;

// Labeloid Interface
// "The Immune System": Validates and transforms code
int labeloid_init(void);
int labeloid_cast(const char *filepath, CastingMode mode);

// Direct Scene Loading (Meow -> Trinity Nodes)
// This effectively replaces the old 'loader_load_file' but routed through
// Labeloid
int labeloid_load_scene(const char *filepath);

#endif

#ifndef PUFU_ENGINE_H
#define PUFU_ENGINE_H

#include "pufu/scene.h"

// Initialize the engine (Graphics + Scene)
// Returns 0 on success
int engine_init(int width, int height, const char *title);

// Shutdown
void engine_cleanup(void);

// Render the current state of the scene
void engine_render(Scene *scene);

// Helper to calculate camera matrices
void engine_update_camera(Scene *scene);

// Helper to get global scene
Scene *engine_get_scene(void);

#endif

#ifndef PUFU_SCENE_H
#define PUFU_SCENE_H

#include "pufu/entity.h"

// Alias for backward compatibility in Trinity
typedef PufuEntity Entity;

// --- Scene ---
typedef struct {
  Entity *head;
  Entity *tail;
  int entity_count;

  // Global State
  Entity *active_camera;
} Scene;

// --- API ---

// Lifecycle
Scene *scene_init(void);
void scene_free(Scene *scene);
void scene_clear(Scene *scene);

// Entity Management
Entity *scene_create_entity(Scene *scene, const char *name);
Entity *scene_find_entity(Scene *scene, const char *name);
void entity_destroy(Scene *scene, Entity *entity);

// Components & Props
void entity_set_pos(Entity *entity, float x, float y, float z);
void entity_set_rot(Entity *entity, float x, float y, float z);
void entity_set_scale(Entity *entity, float x, float y, float z);
int entity_add_component(Entity *entity, ComponentType type, int resource_id);

// Camera
void scene_set_active_camera(Scene *scene, const char *name);

#endif // PUFU_SCENE_H

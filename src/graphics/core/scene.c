#include "pufu/scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int global_entity_id_counter = 1;

Scene *scene_init(void) {
  Scene *s = (Scene *)malloc(sizeof(Scene));
  if (!s)
    return NULL;

  s->head = NULL;
  s->tail = NULL;
  s->entity_count = 0;
  s->active_camera = NULL;
  return s;
}

void scene_free(Scene *scene) {
  if (!scene)
    return;

  scene_clear(scene);
  free(scene);
}

void scene_clear(Scene *scene) {
  if (!scene)
    return;

  PufuEntity *current = scene->head;
  while (current) {
    PufuEntity *next = current->next;
    free(current);
    current = next;
  }

  scene->head = NULL;
  scene->tail = NULL;
  scene->entity_count = 0;
  scene->active_camera = NULL;
}

PufuEntity *scene_create_entity(Scene *scene, const char *name) {
  if (!scene)
    return NULL;

  PufuEntity *e = (PufuEntity *)malloc(sizeof(PufuEntity));
  if (!e)
    return NULL;

  // Init Defaults
  e->id = global_entity_id_counter++;
  strncpy(e->name, name, MAX_NAME_LEN - 1);
  e->name[MAX_NAME_LEN - 1] = '\0';
  e->active = 1;

  e->pos = (Vec3){0.0f, 0.0f, 0.0f};
  e->rot = (Vec3){0.0f, 0.0f, 0.0f};
  e->scale = (Vec3){1.0f, 1.0f, 1.0f};

  e->component_count = 0;
  e->next = NULL;
  e->parent = NULL;
  e->children = NULL;

  // Add to List
  if (!scene->head) {
    scene->head = e;
    scene->tail = e;
  } else {
    scene->tail->next = e;
    scene->tail = e;
  }

  scene->entity_count++;
  printf("Trinity Native: Created PufuEntity '%s' (ID: %d)\n", name, e->id);
  return e;
}

PufuEntity *scene_find_entity(Scene *scene, const char *name) {
  if (!scene || !name)
    return NULL;

  PufuEntity *current = scene->head;
  while (current) {
    if (strncmp(current->name, name, MAX_NAME_LEN) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

void entity_set_pos(PufuEntity *entity, float x, float y, float z) {
  if (!entity)
    return;
  entity->pos.x = x;
  entity->pos.y = y;
  entity->pos.z = z;
}

void entity_set_rot(PufuEntity *entity, float x, float y, float z) {
  if (!entity)
    return;
  entity->rot.x = x;
  entity->rot.y = y;
  entity->rot.z = z;
}

void entity_set_scale(PufuEntity *entity, float x, float y, float z) {
  if (!entity)
    return;
  entity->scale.x = x;
  entity->scale.y = y;
  entity->scale.z = z;
}

int entity_add_component(PufuEntity *entity, ComponentType type,
                         int resource_id) {
  if (!entity || entity->component_count >= MAX_COMPONENTS)
    return -1;

  Component *c = &entity->components[entity->component_count++];
  c->type = type;
  c->resource_id = resource_id;
  // Defaults for params
  c->param1 = (Vec4){0, 0, 0, 0};
  c->param2 = (Vec4){0, 0, 0, 0};

  return 0;
}

void scene_set_active_camera(Scene *scene, const char *name) {
  if (!scene)
    return;
  PufuEntity *e = scene_find_entity(scene, name);
  if (e) {
    scene->active_camera = e;
    printf("Trinity Native: Active camera set to '%s'\n", name);
  } else {
    printf("Trinity Native: Warning - Camera '%s' not found\n", name);
  }
}

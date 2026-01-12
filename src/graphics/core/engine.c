#include "pufu/engine.h"
#include "pufu/graphics.h"
#include <GLES3/gl3.h> // Required for GL_FALSE, GL_TRUE, glDepthMask
#include <math.h>
#include <stdio.h>
#include <string.h>

// Extern renderer state setter
extern void pufu_renderer_set_camera(float *view, float *proj);
extern int screen_width;
extern int screen_height;

// Validar typedef mat4 si no está en header compartido (duplicado temporal)
typedef float mat4[16];
extern void mat4_identity(mat4 m);
extern void mat4_translate(mat4 m, float x, float y, float z);
extern void mat4_multiply(mat4 res, mat4 a, mat4 b);
extern void mat4_perspective(mat4 m, float fov, float aspect, float near,
                             float far);
extern void mat4_look_at(mat4 m, float eyeX, float eyeY, float eyeZ,
                         float centerX, float centerY, float centerZ, float upX,
                         float upY, float upZ);

// Global Scene Singleton
static Scene *g_scene = NULL;

Scene *engine_get_scene(void) { return g_scene; }

int engine_init(int width, int height, const char *title) {
  if (pufu_graphics_init(width, height, title) != 0) {
    return -1;
  }
  // Initialize Global Scene
  g_scene = scene_init();
  if (!g_scene) {
    printf("Trinity Engine: Failed to init global scene.\n");
    return -1;
  }

  // Load Default Primitives (ID 1 = Sphere/Cube placeholder)
  // Load Default Primitives
  pufu_create_primitive(1); // Cube
  pufu_create_primitive(2); // Sphere
  return 0;
}

void engine_cleanup(void) {
  if (g_scene) {
    scene_free(g_scene);
    g_scene = NULL;
  }
  pufu_graphics_cleanup();
}

void engine_render(Scene *scene_arg) {
  Scene *scene = scene_arg ? scene_arg : g_scene;
  if (!scene)
    return;

  // 1. Clear Screen
  pufu_graphics_clear(0.0f, 0.0f, 0.0f); // Black background

  // 2. Setup Camera
  mat4 view, proj;

  if (scene->active_camera) {
    Entity *cam = scene->active_camera;
    mat4_identity(view);

    // Find Camera Component
    Component *cam_c = NULL;
    for (int k = 0; k < cam->component_count; k++) {
      if (cam->components[k].type == COMP_CAMERA) {
        cam_c = &cam->components[k];
        break;
      }
    }

    if (cam_c) {
      // --- Systematic Camera Logic (Pyggel Parity) ---
      // Legacy mapping: param1.x == 1.0 -> LookAt, 0.0 -> LookFrom
      int is_look_at = (int)cam_c->param1.x;
      float distance = cam_c->param2.x;
      (void)distance;

      if (is_look_at) {
        // LookAt Logic (Camera looking AT target)
        // Cam Pos: cam->pos
        // Target: (0,0,0) (Or center of orbit) - logic says distance is radius?
        // Pyggel: cam.set(pos:(0, 0, 5), distance:10, type:"look_at")
        // Usually LookAt takes Eye and Center.
        // If "pos" is the camera position, we just look at 0,0,0?
        // Or if it's orbital, "distance" implies constraints?
        // Let's assume simplest LookAt: Eye=cam->pos, Center=(0,0,0),
        // Up=(0,1,0) Wait, pufu script says: cam2.set(pos:(0, 0, 5),
        // distance:10, type:"look_at") And "cam1.set(pos:(0, 0, -10),
        // type:"look_from")"

        float eye[3] = {cam->pos.x, cam->pos.y, cam->pos.z};
        float center[3] = {0.0f, 0.0f, 0.0f}; // Default look at origin?
        float up[3] = {0.0f, 1.0f, 0.0f};

        mat4_look_at(view, eye[0], eye[1], eye[2], center[0], center[1],
                     center[2], up[0], up[1], up[2]);

      } else {
        // LookFrom Logic (FPS Camera)
        // Legacy: glRotate(...); glTranslatef(-posx, -posy, posz);
        // Note: LookAt can also simulate LookFrom if we define Center = Eye +
        // Forward. But let's keep the translation method if it worked, OR use
        // look_at with a forward vector. Let's stick to the translation method
        // for now to match exactly what we had, or upgrade it? The previous
        // code: mat4_translate(view, -cam->pos.x, -cam->pos.y, cam->pos.z);
        // This is effectively: View = Translation(-Pos). (Assuming no rotation
        // for now)
        mat4_translate(view, -cam->pos.x, -cam->pos.y, cam->pos.z);
      }
    } else {
      // Active entity has no camera component? Fallback to just pos.
      mat4_translate(view, -cam->pos.x, -cam->pos.y, cam->pos.z);
    }
  } else {
    mat4_identity(view);
    mat4_translate(view, 0.0f, 0.0f, -5.0f);
  }

  float aspect = (float)screen_width / (float)screen_height;
  if (aspect == 0)
    aspect = 1.0f; // Protection

  mat4_perspective(proj, 1.047f, aspect, 0.1f, 100.0f); // 60 deg

  pufu_renderer_set_camera(view, proj);

  // --- Render Entities ---
  Entity *current = scene->head;
  while (current) {
    if (current->active) {
      // 3.1 Behaviors First (Scripts)
      for (int i = 0; i < current->component_count; i++) {
        if (current->components[i].type == COMP_SCRIPT) {
          if (current->components[i].resource_id == 1) { // Rotate Y
            current->rot.y += 1.0f;
          } else if (current->components[i].resource_id ==
                     2) { // Follow (Billboard)
            if (scene->active_camera) {
              // Simple Spherical Billboard: Copy Camera Rotation
              // Or just LookAt Camera.
              // Let's copy Y rotation for now (Cylindrical Billboard)
              // current->rot.y = scene->active_camera->rot.y;
              // Actually, standard billboard is inverse of camera view matrix
              // rotation. For simplicity in this engine version: we match
              // camera Y.
              current->rot.y = scene->active_camera->rot.y;
              current->rot.x = scene->active_camera->rot.x; // Full billboard
            }
          }
        }
      }

      // 3.2 Render Components
      for (int i = 0; i < current->component_count; i++) {
        Component *c = &current->components[i];

        if (c->type == COMP_MODEL) {
          // Standard Model Rendering
          int model_id = c->resource_id;
          int tex_id = (int)c->param1.y; // Texture ID stored in Y

          pufu_graphics_draw_model(model_id, tex_id, current->pos.x,
                                   current->pos.y, current->pos.z,
                                   current->scale.x, current->rot.y);
        } else if (c->type == COMP_TEXT) {
          // Text Rendering Placeholder
          // Draw a small cube/quad representing the text anchor
          // TODO: Implement DynamicFontMesh generator
          // Use a debug color (e.g. Magenta if supported, or just ID 1 with no
          // texture) For now, draw as model 1 (Cube)
          pufu_graphics_draw_model(1, 0, current->pos.x, current->pos.y,
                                   current->pos.z, current->scale.x,
                                   current->rot.y);
        } else if (c->type == COMP_SKYBOX) {
          // Skybox Logic
          glDepthMask(GL_FALSE);
          // Position Skybox at Camera Origin
          float cx = scene->active_camera ? scene->active_camera->pos.x : 0;
          float cy = scene->active_camera ? scene->active_camera->pos.y : 0;
          float cz = scene->active_camera ? scene->active_camera->pos.z : 0;

          int tex_id = (int)c->param1.y;

          // Draw Giant Unit Cube (Scale 100.0) to ensure it acts as background
          // Pyggel uses 1.0 but with specific camera matrix hacking.
          // We use simple large box cancellation.
          pufu_graphics_draw_model(1, tex_id, cx, cy, cz, 100.0f, 0.0f);

          glDepthMask(GL_TRUE);
        } else if (c->type == COMP_EMITTER) {
          // Fire Emitter - Render a cluster of particles
          int tex_id = (int)c->param1.y;

          // Pseudo-random cluster based on time/frame? We don't have time
          // passed here easily yet. We will just draw a static "volumetric"
          // cluster for now to look better than a single block.
          float base_x = current->pos.x;
          float base_y = current->pos.y;
          float base_z = current->pos.z;

          // Draw 5 particles rising
          for (int p = 0; p < 5; p++) {
            float offset_y = p * 0.3f;        // Rising
            float scale = 0.3f - (p * 0.05f); // Getting smaller
            // Oscillation (using pos as seed)
            float osc_x = sinf(offset_y * 10.0f + base_x) * 0.2f;

            pufu_graphics_draw_model(1, tex_id, base_x + osc_x,
                                     base_y + offset_y, base_z, scale,
                                     current->rot.y + (p * 45.0f));
          }
        }
      }
    }
    current = current->next;
  }
} // End engine_render

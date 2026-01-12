#include "pufu/graphics.h"
#include <GLES3/gl3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External globals from context.c
extern int screen_width;
extern int screen_height;

// --- 3D Structures (Shared with assets.c via extern if needed, but for now
// redefined or moved to header) --- Ideally this should be in a private header.
// For now, we assume assets.c populates the global array and we access it here.
// To avoid duplication, let's define the struct in a common internal header or
// just duplicate for this step if we don't want to create another file. Better
// approach: Create src/core/graphics/internal.h

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  int index_count;
  int vertex_count;
} Model;

#define MAX_MODELS 16
extern Model models[MAX_MODELS];
extern int model_count;

// --- Shaders ---

const char *vertex_shader_3d_src =
    "#version 300 es\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "layout(location = 1) in vec2 a_tex;\n"
    "out vec2 v_tex;\n"
    "uniform mat4 u_mvp;\n"
    "void main() {\n"
    "    gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
    "    v_tex = a_tex;\n"
    "}\n";

const char *fragment_shader_3d_src =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_tex;\n"
    "out vec4 color;\n"
    "uniform sampler2D u_tex;\n"
    "void main() {\n"
    "    vec4 texColor = texture(u_tex, v_tex);\n"
    "    // if(texColor.a < 0.1) discard;\n"
    "    color = texColor;\n"
    "}\n";

const char *vertex_shader_src =
    "#version 300 es\n"
    "layout(location = 0) in vec2 a_pos;\n"
    "layout(location = 1) in vec2 a_tex;\n"
    "out vec2 v_tex;\n"
    "uniform vec2 u_res;\n"
    "uniform vec4 u_rect;\n" // x, y, w, h
    "void main() {\n"
    "    vec2 pos = a_pos * u_rect.zw + u_rect.xy;\n"
    "    // Convert to Clip Space (-1 to 1)\n"
    "    // 0,0 is top-left in Pufu, but OpenGL is bottom-left. \n"
    "    // Let's map 0,0 to top-left.\n"
    "    float x = (pos.x / u_res.x) * 2.0 - 1.0;\n"
    "    float y = 1.0 - (pos.y / u_res.y) * 2.0;\n"
    "    gl_Position = vec4(x, y, 0.0, 1.0);\n"
    "    v_tex = a_tex;\n"
    "}\n";

const char *fragment_shader_src = "#version 300 es\n"
                                  "precision mediump float;\n"
                                  "in vec2 v_tex;\n"
                                  "out vec4 color;\n"
                                  "uniform sampler2D u_tex;\n"
                                  "void main() {\n"
                                  "    color = texture(u_tex, v_tex);\n"
                                  "}\n";

static GLuint shader_program;
static GLuint shader_program_3d;
static GLuint vao, vbo;
static GLint u_res_loc, u_rect_loc;
static GLint u_mvp_loc;

// Helper to compile shader
GLuint compile_shader(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info[512];
    glGetShaderInfoLog(shader, 512, NULL, info);
    printf("Trinity Error: Shader compilation failed: %s\n", info);
    return 0;
  }
  return shader;
}

void pufu_renderer_init(void) {
  // Fix: Set Viewport
  glViewport(0, 0, screen_width, screen_height);

  // --- Init 2D Shaders ---
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
  shader_program = glCreateProgram();
  glAttachShader(shader_program, vs);
  glAttachShader(shader_program, fs);
  glLinkProgram(shader_program);

  GLint success;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    char info[512];
    glGetProgramInfoLog(shader_program, 512, NULL, info);
    printf("Trinity Error: Shader program linking failed: %s\n", info);
  }
  glDeleteShader(vs);
  glDeleteShader(fs);

  u_res_loc = glGetUniformLocation(shader_program, "u_res");
  u_rect_loc = glGetUniformLocation(shader_program, "u_rect");

  // --- Init 3D Shaders ---
  GLuint vs3d = compile_shader(GL_VERTEX_SHADER, vertex_shader_3d_src);
  GLuint fs3d = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_3d_src);
  shader_program_3d = glCreateProgram();
  glAttachShader(shader_program_3d, vs3d);
  glAttachShader(shader_program_3d, fs3d);
  glLinkProgram(shader_program_3d);

  glGetProgramiv(shader_program_3d, GL_LINK_STATUS, &success);
  if (!success) {
    char info[512];
    glGetProgramInfoLog(shader_program_3d, 512, NULL, info);
    printf("Trinity Error: 3D Shader program linking failed: %s\n", info);
  }
  glDeleteShader(vs3d);
  glDeleteShader(fs3d);

  u_mvp_loc = glGetUniformLocation(shader_program_3d, "u_mvp");

  // Enable Depth Test
  glEnable(GL_DEPTH_TEST);
  // glDisable(GL_CULL_FACE);

  // Quad Vertices (0,0 to 1,1)
  float vertices[] = {// Pos      // Tex
                      0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Enable Alpha Blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void pufu_renderer_cleanup(void) {
  // Cleanup GL resources if needed
}

void pufu_graphics_clear(float r, float g, float b) {
  (void)r;
  (void)g;
  (void)b;
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);               // Standard Grey
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Depth too!
}

void pufu_graphics_draw_texture(int texture_id, float x, float y, float w,
                                float h) {
  glUseProgram(shader_program);

  glUniform2f(u_res_loc, (float)screen_width, (float)screen_height);
  glUniform4f(u_rect_loc, x, y, w, h);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, (GLuint)texture_id);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// --- Matrix Math Helpers ---
typedef float mat4[16];

void mat4_identity(mat4 m) {
  for (int i = 0; i < 16; i++)
    m[i] = 0.0f;
  m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void mat4_multiply(mat4 res, mat4 a, mat4 b) {
  mat4 temp;
  for (int c = 0; c < 4; c++) {
    for (int r = 0; r < 4; r++) {
      temp[c * 4 + r] = 0.0f;
      for (int k = 0; k < 4; k++) {
        temp[c * 4 + r] += a[k * 4 + r] * b[c * 4 + k];
      }
    }
  }
  for (int i = 0; i < 16; i++)
    res[i] = temp[i];
}

void mat4_perspective(mat4 m, float fov, float aspect, float near, float far) {
  mat4_identity(m);
  float tan_half_fov = tanf(fov / 2.0f);
  m[0] = 1.0f / (aspect * tan_half_fov);
  m[5] = 1.0f / tan_half_fov;
  m[10] = -(far + near) / (far - near);
  m[11] = -1.0f;
  m[14] = -(2.0f * far * near) / (far - near);
  m[15] = 0.0f;
}

void mat4_translate(mat4 m, float x, float y, float z) {
  mat4 t;
  mat4_identity(t);
  t[12] = x;
  t[13] = y;
  t[14] = z;
  mat4_multiply(m, m, t);
}

void mat4_scale(mat4 m, float s) {
  mat4 t;
  mat4_identity(t);
  t[0] = s;
  t[5] = s;
  t[10] = s;
  mat4_multiply(m, m, t);
}

void mat4_rotate(mat4 m, float angle, float x, float y, float z) {
  float rad = angle * 3.14159f / 180.0f;
  float c = cosf(rad);
  float s = sinf(rad);
  float c1 = 1.0f - c;

  // Normalize axis
  float len = sqrtf(x * x + y * y + z * z);
  if (len < 0.0001f)
    return;
  x /= len;
  y /= len;
  z /= len;

  mat4 r;
  mat4_identity(r);

  r[0] = x * x * c1 + c;
  r[1] = x * y * c1 - z * s;
  r[2] = x * z * c1 + y * s;

  r[4] = y * x * c1 + z * s;
  r[5] = y * y * c1 + c;
  r[6] = y * z * c1 - x * s;

  r[8] = z * x * c1 - y * s;
  r[9] = z * y * c1 + x * s;
  r[10] = z * z * c1 + c;

  mat4_multiply(m, m, r);
}

void mat4_look_at(mat4 m, float eyeX, float eyeY, float eyeZ, float centerX,
                  float centerY, float centerZ, float upX, float upY,
                  float upZ) {
  float f[3] = {centerX - eyeX, centerY - eyeY, centerZ - eyeZ};
  // Normalize f
  float f_len = sqrtf(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
  if (f_len > 0) {
    f[0] /= f_len;
    f[1] /= f_len;
    f[2] /= f_len;
  }

  // s = f x up
  float s[3] = {f[1] * upZ - f[2] * upY, f[2] * upX - f[0] * upZ,
                f[0] * upY - f[1] * upX};
  // Normalize s
  float s_len = sqrtf(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
  if (s_len > 0) {
    s[0] /= s_len;
    s[1] /= s_len;
    s[2] /= s_len;
  }

  // u = s x f
  float u[3] = {s[1] * f[2] - s[2] * f[1], s[2] * f[0] - s[0] * f[2],
                s[0] * f[1] - s[1] * f[0]};

  mat4 res;
  mat4_identity(res);

  res[0] = s[0];
  res[4] = s[1];
  res[8] = s[2];

  res[1] = u[0];
  res[5] = u[1];
  res[9] = u[2];

  res[2] = -f[0];
  res[6] = -f[1];
  res[10] = -f[2];

  res[12] = -(s[0] * eyeX + s[1] * eyeY + s[2] * eyeZ);
  res[13] = -(u[0] * eyeX + u[1] * eyeY + u[2] * eyeZ);
  res[14] = -(-f[0] * eyeX - f[1] * eyeY - f[2] * eyeZ);

  // Multiply into m (usually m is identity or view, but for lookAt we usually
  // SET it)
  // Assuming m is output. If we follow convention mat4_translate(m, ...)
  // multiplies m * T, then we should probably do m * LookAt. But LookAt is
  // usually absolute. Let's just replacing m if we want standard behavior, OR
  // follow the multiply pattern.
  // Standard glm::lookAt returns a matrix.
  // This function will SET m to the lookAt matrix.
  for (int i = 0; i < 16; i++)
    m[i] = res[i];
}

// --- Global State ---
static mat4 g_view_matrix;
static mat4 g_proj_matrix;

void pufu_renderer_set_camera(float *view, float *proj) {
  if (view)
    memcpy(g_view_matrix, view, sizeof(mat4));
  else
    mat4_identity(g_view_matrix);

  if (proj)
    memcpy(g_proj_matrix, proj, sizeof(mat4));
  else
    mat4_identity(g_proj_matrix);
}

// Note: signature expects texture_id now. We'll need to update header/caller.
// Added ry for rotation
void pufu_graphics_draw_model(int model_id, int texture_id, float x, float y,
                              float z, float scale, float ry) {
  if (model_id < 0 || model_id >= model_count)
    return;

  Model *m = &models[model_id];

  glUseProgram(shader_program_3d);

  // MVP Calculation
  mat4 model, mvp;
  mat4_identity(model);
  mat4_translate(model, x, y, z);
  // Manual Rotate Y using generic generic rorator
  if (ry != 0.0f) {
    mat4_rotate(model, ry, 0.0f, 1.0f, 0.0f);
  }

  mat4_scale(model, scale);

  // Use global matrices (set by Scene Engine)
  mat4_multiply(mvp, g_view_matrix, model);
  mat4_multiply(mvp, g_proj_matrix, mvp);

  glUniformMatrix4fv(u_mvp_loc, 1, GL_FALSE, mvp);

  // Bind Texture
  glActiveTexture(GL_TEXTURE0);
  if (texture_id >= 0) {
    glBindTexture(GL_TEXTURE_2D, (GLuint)texture_id);
  } else {
    // Bind a white texture or 0 (black in some drivers, 0 usually reserves)
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  glBindVertexArray(m->vao);
  if (m->ebo) {
    glDrawElements(GL_TRIANGLES, m->index_count, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(GL_TRIANGLES, 0, m->vertex_count);
  }
}

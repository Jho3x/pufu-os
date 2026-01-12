#include "pufu/graphics.h"
#include <GLES3/gl3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

// --- 3D Structures ---
typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  int index_count;
  int vertex_count;
} Model;

#define MAX_MODELS 16
Model models[MAX_MODELS];
int model_count = 0;

// Basic Cube Vertices
float cube_vertices[] = {
    // Back face
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // Bottom-left
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // bottom-right
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // top-left
    // Front face
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // bottom-right
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // top-left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
    // Left face
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-left
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
                                     // Right face
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   // bottom-left
    // Bottom face
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f,  // top-left
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
    // Top face
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-right
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f   // bottom-left
};

void pufu_create_primitive(int id) {
  if (id == 1) {
    // CUBE
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices,
                 GL_STATIC_DRAW);

    // Pos: 3 floats, stride: 5 * float
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Tex: 2 floats, offset: 3 * float
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    if (id < MAX_MODELS) {
      models[id].vao = vao;
      models[id].vbo = vbo;
      models[id].ebo = 0;
      models[id].index_count = 0;
      models[id].vertex_count = 36;
      if (model_count <= id)
        model_count = id + 1;
      printf("Trinity: Created primitive CUBE ID %d\n", id);
    }
  } else if (id == 2) {
    // SPHERE
    int rings = 20;
    int sectors = 20;
    // int vertex_count = rings * sectors * 6; // approximate for lines or tris
    // Actually we need vertices = (rings+1) * (sectors+1)
    // Indices depend on how we draw.
    // Let's do simple separate triangles for simplicity (no indices needed
    // strictly, but harder)
    // Let's do indexed for sphere, it's cleaner.

    int num_vertices = (rings + 1) * (sectors + 1);
    int num_indices = rings * sectors * 6;

    float *vertices = (float *)malloc(num_vertices * 5 * sizeof(float));
    unsigned int *indices =
        (unsigned int *)malloc(num_indices * sizeof(unsigned int));

    float R = 0.5f; // Radius
    int v_idx = 0;

    for (int r = 0; r <= rings; r++) {
      float v = (float)r / (float)rings; // 0 to 1
      float phi = v * 3.14159f;          // 0 to PI

      for (int s = 0; s <= sectors; s++) {
        float u = (float)s / (float)sectors; // 0 to 1
        float theta = u * 2.0f * 3.14159f;   // 0 to 2PI

        float x = cosf(theta) * sinf(phi);
        float y = cosf(phi);
        float z = sinf(theta) * sinf(phi);

        // Pos
        vertices[v_idx++] = x * R;
        vertices[v_idx++] = y * R;
        vertices[v_idx++] = z * R;
        // Tex
        vertices[v_idx++] = u;
        vertices[v_idx++] = v;
      }
    }

    int i_idx = 0;
    for (int r = 0; r < rings; r++) {
      for (int s = 0; s < sectors; s++) {
        int next_row = (r + 1) * (sectors + 1);
        int current_row = r * (sectors + 1);

        // Triangle 1
        indices[i_idx++] = current_row + s;
        indices[i_idx++] = next_row + s;
        indices[i_idx++] = current_row + s + 1;

        // Triangle 2
        indices[i_idx++] = current_row + s + 1;
        indices[i_idx++] = next_row + s;
        indices[i_idx++] = next_row + s + 1;
      }
    }

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * 5 * sizeof(float), vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned int),
                 indices, GL_STATIC_DRAW);

    // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Tex
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    free(vertices);
    free(indices);

    if (id < MAX_MODELS) {
      models[id].vao = vao;
      models[id].vbo = vbo;
      models[id].ebo = ebo;
      models[id].index_count = num_indices;
      models[id].vertex_count = num_vertices;
      if (model_count <= id)
        model_count = id + 1;
      printf("Trinity: Created primitive SPHERE ID %d\n", id);
    }
  }
}

// --- Texture Implementation ---

int pufu_graphics_load_texture(const char *path, int *w, int *h) {
  int width, height, nrChannels;
  int is_fallback = 0;
  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

  // Fallback if missing
  if (!data) {
    printf("Trinity Warning: Failed to load %s. Generating fallback texture.\n",
           path);
    width = 2;
    height = 2;
    nrChannels = 4; // Use RGBA for alignment and safety
    data = (unsigned char *)malloc(width * height * nrChannels);
    // Magenta/Black checkerboard (RGBA)
    unsigned char colors[16] = {255, 0, 255, 255, 0,   0, 0,   255,
                                0,   0, 0,   255, 255, 0, 255, 255};
    memcpy(data, colors, 16);
    is_fallback = 1;
  }

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Use nearest neighbor for crisp checkerboard
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);

  if (!is_fallback) {
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
  } else {
    free(data);
  }

  if (w)
    *w = width;
  if (h)
    *h = height;

  printf("Trinity: Loaded texture %s (%dx%d) ID: %d\n", path, width, height,
         texture);
  return (int)texture;
}

// --- 3D Model Implementation ---

int pufu_graphics_load_model(const char *path) {
  if (model_count >= MAX_MODELS)
    return -1;

  cgltf_options options = {0};
  cgltf_data *data = NULL;
  cgltf_result result = cgltf_parse_file(&options, path, &data);

  if (result != cgltf_result_success) {
    printf("Trinity Error: Failed to parse GLB %s\n", path);
    return -1;
  }

  result = cgltf_load_buffers(&options, data, path);
  if (result != cgltf_result_success) {
    printf("Trinity Error: Failed to load buffers for %s\n", path);
    cgltf_free(data);
    return -1;
  }

  // Assume first mesh and first primitive for simplicity
  if (data->meshes_count == 0) {
    printf("Trinity Error: No meshes in %s\n", path);
    cgltf_free(data);
    return -1;
  }

  cgltf_mesh *mesh = &data->meshes[0];
  cgltf_primitive *prim = &mesh->primitives[0];

  // Find POSITION and TEXCOORD_0 attributes
  cgltf_accessor *pos_acc = NULL;
  cgltf_accessor *tex_acc = NULL;

  for (size_t i = 0; i < prim->attributes_count; i++) {
    if (prim->attributes[i].type == cgltf_attribute_type_position) {
      pos_acc = prim->attributes[i].data;
    } else if (prim->attributes[i].type == cgltf_attribute_type_texcoord) {
      tex_acc = prim->attributes[i].data;
    }
  }

  if (!pos_acc) {
    printf("Trinity Error: No POSITION attribute in %s\n", path);
    cgltf_free(data);
    return -1;
  }

  // Create VAO
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // VBO for Position (loc 0)
  GLuint vbo_pos;
  glGenBuffers(1, &vbo_pos);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);

  int vertex_count = pos_acc->count;
  float *vertices = (float *)malloc(vertex_count * 3 * sizeof(float));
  for (int i = 0; i < vertex_count; i++) {
    cgltf_accessor_read_float(pos_acc, i, &vertices[i * 3], 3);
  }
  glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), vertices,
               GL_STATIC_DRAW);
  free(vertices);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // VBO for TexCoord (loc 1)
  // Even if no texture, we should enable something or disable attrib 1 to avoid
  // crash? Ideally provide default UVs (0,0) if missing.
  GLuint vbo_tex = 0;
  glGenBuffers(1, &vbo_tex);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_tex);

  float *uvs = (float *)malloc(vertex_count * 2 * sizeof(float));
  if (tex_acc) {
    for (int i = 0; i < vertex_count; i++) {
      cgltf_accessor_read_float(tex_acc, i, &uvs[i * 2], 2);
    }
  } else {
    // Zero UVs
    memset(uvs, 0, vertex_count * 2 * sizeof(float));
  }
  glBufferData(GL_ARRAY_BUFFER, vertex_count * 2 * sizeof(float), uvs,
               GL_STATIC_DRAW);
  free(uvs);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);

  // We assign vbo_pos to model->vbo just to keep struct happy, but really we
  // have multiple VBOs now. Model struct only has one vbo field. We should just
  // ignore it or update struct. Since we rely on VAO, VBO IDs are not strictly
  // needed unless we delete them. For cleanup we need them. For now, let's
  // assign vbo_pos.
  GLuint vbo = vbo_pos;

  // Handle Indices if present
  GLuint ebo = 0;
  int index_count = 0;
  if (prim->indices) {
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    index_count = prim->indices->count;
    unsigned int *indices =
        (unsigned int *)malloc(index_count * sizeof(unsigned int));
    for (int i = 0; i < index_count; i++) {
      indices[i] = cgltf_accessor_read_index(prim->indices, i);
    }

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned int),
                 indices, GL_STATIC_DRAW);
    free(indices);
  } else {
    index_count = pos_acc->count; // DrawArrays
  }

  cgltf_free(data);

  models[model_count].vao = vao;
  models[model_count].vbo = vbo;
  models[model_count].ebo = ebo;
  models[model_count].index_count = (prim->indices) ? index_count : 0;
  models[model_count].vertex_count = pos_acc->count;

  printf("Trinity: Loaded model %s (VAO: %d, Vtx: %d)\n", path, vao,
         (int)pos_acc->count);
  return model_count++;
}

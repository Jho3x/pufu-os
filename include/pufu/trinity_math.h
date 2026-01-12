#ifndef TRINITY_MATH_H
#define TRINITY_MATH_H

#include <math.h>

#define PI 3.14159265359f
#define DEG2RAD(x) ((x) * PI / 180.0f)

typedef struct {
  float x, y, z;
} vec3;

typedef struct {
  float r, g, b, a;
} vec4;

typedef struct {
  float m[4][4];
} mat4;

// --- Funciones Básicas ---
vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_scale(vec3 v, float s);
float vec3_dot(vec3 a, vec3 b);
vec3 vec3_cross(vec3 a, vec3 b);
float vec3_len(vec3 v);
vec3 vec3_norm(vec3 v);

mat4 mat4_identity();
mat4 mat4_translate(mat4 m, vec3 v);
mat4 mat4_scale(mat4 m, vec3 v);

#endif // T_MATH_H

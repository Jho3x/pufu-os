#include "trinity_math.h"
#include <string.h>

vec3 vec3_add(vec3 a, vec3 b) {
  vec3 r = {a.x + b.x, a.y + b.y, a.z + b.z};
  return r;
}

vec3 vec3_sub(vec3 a, vec3 b) {
  vec3 r = {a.x - b.x, a.y - b.y, a.z - b.z};
  return r;
}

vec3 vec3_scale(vec3 v, float s) {
  vec3 r = {v.x * s, v.y * s, v.z * s};
  return r;
}

float vec3_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

vec3 vec3_cross(vec3 a, vec3 b) {
  vec3 r;
  r.x = a.y * b.z - a.z * b.y;
  r.y = a.z * b.x - a.x * b.z;
  r.z = a.x * b.y - a.y * b.x;
  return r;
}

float vec3_len(vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

vec3 vec3_norm(vec3 v) {
  float len = vec3_len(v);
  if (len > 0)
    return vec3_scale(v, 1.0f / len);
  return v;
}

mat4 mat4_identity() {
  mat4 m;
  memset(m.m, 0, sizeof(m.m));
  m.m[0][0] = 1.0f;
  m.m[1][1] = 1.0f;
  m.m[2][2] = 1.0f;
  m.m[3][3] = 1.0f;
  return m;
}

mat4 mat4_translate(mat4 m, vec3 v) {
  mat4 r = m;
  r.m[3][0] += v.x;
  r.m[3][1] += v.y;
  r.m[3][2] += v.z;
  return r;
}

mat4 mat4_scale(mat4 m, vec3 v) {
  mat4 r = m;
  r.m[0][0] *= v.x;
  r.m[1][1] *= v.y;
  r.m[2][2] *= v.z;
  return r;
}

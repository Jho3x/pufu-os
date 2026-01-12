#ifndef GRAPHICS_LOADER_H
#define GRAPHICS_LOADER_H

#include "pufu/scene.h"

// Cargar archivo Trinity Scene
int loader_load_file(Scene *scene, const char *filepath);

// Parsear una línea
int loader_parse_line(Scene *scene, char *line);

#endif // GRAPHICS_LOADER_H

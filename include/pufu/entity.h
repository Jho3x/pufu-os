#ifndef PUFU_ENTITY_H
#define PUFU_ENTITY_H

#include <stddef.h>

#define MAX_NAME_LEN 32
#define MAX_COMPONENTS 8

// --- Math Types (Shared) ---
// Defined here to avoid cyclic deps, or should be in pufu/math.h?
// For now, let's keep them here or assume pufu/math.h exists.
// Given previous context, we have src/math, but maybe no global pufu/math.h yet.
// Ensuring we have basic vector types available.

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y, z, w;
} Vec4;

// --- Component System (ECS) ---
// This enum defines the "brain" or "skill" slots an entity can have.
typedef enum {
    COMP_NONE = 0,
    COMP_MODEL,     // 3D Model (GLB/GLTF/Primitive)
    COMP_LIGHT,     // Light Source
    COMP_CAMERA,    // Camera parameters
    COMP_SCRIPT,    // Logic/Behavior
    COMP_EMITTER,   // Particle System
    COMP_SKYBOX,    // Skybox specific
    COMP_TEXT,      // Text Rendering
    COMP_IMAGE2D    // 2D Image Rendering
} ComponentType;

typedef struct {
    ComponentType type;
    int resource_id;       // ID used by the specific subsystem (Loader/Renderer)
    
    // Generic data storage for simple components
    // Complex components (like massive particle arrays) might use resource_id to look up data in a manager.
    Vec4 param1; 
    Vec4 param2;
    void *data;            // Pointer for extension/complex structures
} Component;

// --- The Global Pufu Object ---
typedef struct PufuEntity {
    int id;
    char name[MAX_NAME_LEN];
    int active;
    
    // --- The Physical Kernel (Truth) ---
    Vec3 pos;
    Vec3 rot; // Euler angles
    Vec3 scale;
    
    // --- The Capabilities (Skills) ---
    Component components[MAX_COMPONENTS];
    int component_count;
    
    // --- Hierarchy ---
    struct PufuEntity *parent;
    struct PufuEntity *children;
    struct PufuEntity *next; // Linked list sibling
    
} PufuEntity;

#endif // PUFU_ENTITY_H

# Meow Language Dictionary (v1.0)

**Source of Truth**: `z_pufu_0/trinity_demo.pufu`, `z_pufu_0/objetos.md`

## 1. Syntax Overview
Meow is an imperative, object-oriented scripting language that transpiles to Paw (Assembly). It is designed to be the "Face" of Pufu: readable and expressive.

### 1.1 Declarations
Assigning a string literal to a variable instantiates or references an object.
```meow
variable_name = "ObjectIdentifier"
```
*   `engranaje = "gear"` (Creates a handle to system object "gear")
*   `cubo = "cube"` (Creates a new entity/handle "cube")

### 1.2 Method Calls
Invoking actions on objects using Dot Notation.
```meow
variable.method(key1: value1, key2: value2)
```
*   `cubo.set(pos: (0, 0, 0), color: "red")`

---

## 2. System Objects

### Gear (`engranaje`)
The core engine loop.
- **Methods**:
    - `init()`: Starts the main loop.
    - `set(window: "name", space: "name")`: Binds the active window and space.

### Window (`ventana`)
The display surface.
- **Methods**:
    - `set(rectangle: "W*H", space: "name")`: Configures size and attached space.

### Space (`espacio`)
The scene graph container.
- **Methods**:
    - `add("obj1, obj2, ...")`: Registers entities into the scene.
    - `set(camera: "name")`: Sets the active camera.

---

## 3. Scene Objects (Entities)

### Base Properties (All Entities)
- `pos`: Vector3 `(x, y, z)`
- `rot`: Vector3 `(rx, ry, rz)`
- `scale`: Vector3 `(sx, sy, sz)` or Scalar `s`

### Primitive / Polytive
- **Types**: `skymesh`, `sphere`, `cube`.
- **Properties**:
    - `type`: Geometry type string.
    - `texture`: Path to texture image.
    - `color`: String color name (e.g., "fuchsia", "white") or Hex.

### Light (`luz`)
- **Properties**:
    - `color`: Light color.
    - `intensity`: Float.

### Emitter (`fuego`)
- **Properties**:
    - `type`: Preset name (e.g., "fire").

### Text (`texto`)
3D Text labels.
- **Properties**:
    - `string`: Content text.
    - `type`: Behavior (e.g., "follow" for billboarding).

### Camera (`camara`)
- **Properties**:
    - `view`: Look-at target `(x, y, z)`.
    - `fov`: Field of view.

---

## 4. UI Objects (Extension for Phase 16)
To support 2D UI (Taskbar), we extend the Entity model to UI Elements.

### UI Element (`button`, `frame`)
- **Properties**:
    - `rect`: `(x, y, w, h)` Screen coordinates.
    - `color`: Background color.
    - `label`: Button text.
    - `onclick`: Command string to spawn/exec.

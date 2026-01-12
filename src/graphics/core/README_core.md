# Pufu Core

This directory contains the essential subsystems of the Pufu OS that bridge the gap between the Kernel/VM and the actual hardware/graphics.

## Graphic Subsystem (`src/core/graphics`)
*   **`engine.c`**: High-level graphics engine. Manages the scene graph, camera, and main rendering loop. The `engine_render` function is the heart of the visual output.
*   **`renderer.c`**: Low-level abstraction over OpenGL ES. Handles draw calls, shader management, and buffer updates.
*   **`loader.c`**: Logic for loading assets and parsing procedural scene descriptions (often used by Trinity).
*   **`scene.c`**: Manages the hierarchy of `Entity` objects in the world.
*   **`assets.c`**: Handles loading of external assets like GLTF models and PNG textures.

## Trinity UI System (`src/core/trinity_*`)
*   **`trinity_core.c`**: The central coordinator for the Trinity UI system. Maintains the global UI state.
*   **`trinity_nodes.c`**: Implements the creation and property management of UI nodes (Windows, Buttons, Images).
*   **`trinity_render.c`**: Specialized rendering logic for 2D UI elements, including text (using `stb_truetype`) and vector icons (using `nanosvg`).
*   **`trinity_events.c`**: Event dispatching system for UI interactions (clicks, hover).
*   **`meow_parser.c`**: The parser for the Meow UI declaration language (`.meow`).

## System Utilities
*   **`virtual_bus.c`**: Implements the Virtual Bus for inter-process communication (IPC) between nodes.
*   **`terminal.c`**: A virtual terminal emulator for CLI output within the graphical environment.
*   **`labeloid.c`**: (Legacy/Experimental) A hybrid parser component.

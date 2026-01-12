# Pufu Virtual Machine (VM)

This directory contains the core implementation of the Pufu Virtual Machine, which executes `.pufu` nodes.

## Files

*   **`entry.c`**: The main entry point for the Pufu VM executable (CLI). Handles argument parsing and initializes the primary node execution.
*   **`node_core.c`**: Core logic for managing `PufuNode` structures, including type detection, creation, and lifecycle management.
*   **`node_exec.c`**: The execution engine. Contains the `pufu_node_execute` function which steps through instructions (for Assembler nodes) or updates/rendering logic (for Trinity nodes).
*   **`parser.c`**: The parser for `.pufu` files. It converts the text-based node definitions into in-memory structures (`PufuNode` and `PufuInstruction`).

## Architecture

The VM treats every executing unit as a "Node". Nodes can be of various types:
*   **Assembler (`.pufu`)**: Contains low-level instructions (MOV, ADD, CALL, SPAWN) interpreted by `node_exec.c`.
*   **Trinity (`.meow`, `.trinity`)**: Graphical nodes managed by the Trinity Core engine (linked via `src/core`).
*   **Crystal**: Compiled/Native plugins.

The execution model is tick-based for graphical/service nodes and instruction-based for script nodes.

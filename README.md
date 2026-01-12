# Pufu OS (v0.7 'Lumina')

**Pufu OS** is an experimental, fractal operating system built on a custom Virtual Machine architecture.

> **"Fractal Architecture?"**
> In Pufu OS, the user interface and system structure are recursive. Every window ("Frame") can contain other Frames indefinitely, and they all share the same logic (Trinity Nodes). This allows for infinite nesting and zooming of workspaces.

## 📚 Documentation

Detailed specifications for the custom languages and protocols:

### 1. Languages & Protocols
*   **[Paw (ASM)](docs/specs/paw_dictionary.md)**: The Assembly language for the Pufu VM. Defines the instruction set.
*   **[Meow (UI)](docs/specs/meow_dictionary.md)**: The Markup Language for defining the Fractal UI (Trinity).
*   **[Claw (Kernel)](docs/specs/claw_dictionary.md)**: The Interface Description Language for Kernel Syscalls and IPC.

### 2. Guides
*   **[Cloud Deployment](docs/CLOUD_DEPLOY.md)**: How to run Pufu OS in Google Colab (Headless/Web Mode).

## 🛠️ Prerequisites

To build and run Pufu OS, you need a standard Linux environment:
*   `gcc` (Build Essential)
*   `make`
*   `python3` (For build tools and launcher)

## 🚀 Quick Start

Run the OS in local mode:
```bash
make run
```

Or deploy to the cloud (Google Colab):
```bash
python3 tools/pufu_colab.py
```

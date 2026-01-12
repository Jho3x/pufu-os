# Pufu OS (v0.7 'Lumina')

**Pufu OS** is an experimental, fractal operating system built on a custom Virtual Machine architecture.

## 📚 Documentation Flow

If you are an AI agent or developer looking to understand the system, follow this reading order:

### 1. High-Level Concepts
*   [Architecture](file:///home/joel/z_pufu_4/docs/architecture.md): Explains the Fractal UI, Event-Driven MVC, and "Frame" concepts.
*   [Terminal Architecture (Oxide)](file:///home/joel/z_pufu_4/docs/oxide_reference.md): Explains TWS (Window Spaces), Async CLI, and Headless Apps.

### 2. Technical References
*   [Socket Reference (HAL)](file:///home/joel/z_pufu_4/docs/socket_reference.md): "The Hardware". Defines the `PufuSocket` structs, Hot Swap mechanism, and API dictionary.
*   [Paw Language (ASM)](file:///home/joel/z_pufu_4/docs/paw_reference.md): "The Assembly". Dictionary of instructions for the Pufu VM.

### 3. Implementation Details
*   [Directory Structure](file:///home/joel/z_pufu_4/docs/directory_structure.md): **Start Here**. Detailed breakdown of every folder and key file in the repo.
*   [MVP Plan](file:///home/joel/z_pufu_4/docs/mvp_plan.md): Historical roadmap of the "Pure VM" refactor.
*   [Hot Swap Spec](file:///home/joel/z_pufu_4/docs/hot_swap_spec.md): Specification for the dynamic reload system.

## 🚀 Quick Start
Run the OS (booting to Oxide Shell):
```bash
make run
```

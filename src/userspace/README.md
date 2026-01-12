# Pufu Userspace

This directory contains the code running "inside" the Pufu OS environment. These are `.pufu` scripts and associated assets that make up the user experience.

## Directory Structure

*   **`boot/`**: Critical system startup scripts.
    *   `bootloader.pufu`: The first node executed by the VM. Initializes the environment.
    *   `system.pufu`: The init process that spawns essential services.
    *   `virtual_bus.pufu`: The message bus broker.
*   **`services/`**: Background services that maintain system state.
    *   `session_manager.pufu`: Handles user login and session state.
    *   `task_manager.pufu`: Monitors running processes.
    *   `window_manager.pufu`: Manages window focus and layout (if active).
*   **`apps/`**: Interactive user applications.
    *   `liquid/`: The "Liquid" Desktop Environment (Taskbar, Desktop, Wallpaper).
    *   `trinity_demo.pufu`: A demo application showcasing 3D capabilities.
    *   `oxide.pufu`: A CLI shell environment.

## Boot Sequence
The system startup follows a strict chain of nodes:

1.  **The Spark**: The VM loads `bootloader.pufu`.
2.  **Supervisor**: `bootloader` spawns `task_manager.pufu` and exits. The Task Manager becomes the persistent system guardian.
3.  **Nervous System**: `task_manager` spawns `virtual_bus.pufu` to enable IPC.
4.  **Init**: `task_manager` spawns `system.pufu` to load core services.
5.  **Session**: `system.pufu` spawns `session_manager.pufu`.
6.  **User Environment**: `session_manager` authenticates the user and checks `user_config.pufu` to launch either the **Liquid Desktop** (GUI) or **Oxide Shell** (CLI).

## Philosophy
Userspace code is written in Pufu Assembly (`.pufu`) or the Meow UI language (`.meow`). It relies on System Calls to interact with the Kernel/Core level.

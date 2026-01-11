CC = gcc
# Optimization Flags
CFLAGS = -I./include -Wall -Wextra -O2

LDFLAGS = -ldl -rdynamic

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Sources
BOOTLOADER_SRC = src/vm/entry.c
# Sources
BOOTLOADER_SRC = src/vm/entry.c
CORE_SRCS = src/vm/entry.c \
            src/vm/node_core.c \
            src/vm/node_exec.c \
            src/vm/parser.c \
            src/ipc/virtual_bus.c \
            src/system/hot_reload.c \
            src/system/watchdog.c \
            src/hal/dyn_loader.c \
            src/graphics/core/context.c \
            src/graphics/core/assets.c src/graphics/core/renderer.c \
            src/graphics/core/scene.c src/graphics/core/engine.c src/graphics/core/loader.c \
            src/graphics/trinity/trinity_core.c src/graphics/trinity/trinity_nodes.c \
            src/graphics/trinity/trinity_events.c src/graphics/trinity/trinity_render.c \
            src/graphics/backend/opengl_es_backend.c src/system/labeloid.c \
            src/system/meow_parser.c \
            src/system/terminal.c src/kernel/dispatch.c \
            src/kernel/syscalls/sys_core.c src/kernel/syscalls/sys_process.c \
            src/kernel/syscalls/sys_trinity.c src/kernel/syscalls/sys_ipc.c \
            src/system/crystal.c src/system/logger.c src/system/task_manager.c
# ARM Socket is now a driver, not part of core
# ARM_SRCS = src/hal/arm/arm_socket.c

# Archivos objeto
CORE_OBJS = $(CORE_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
# ARM_OBJS = $(ARM_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

BOOTLOADER_OBJ = $(OBJ_DIR)/core/bootloader.o

# Binarios
NODE = $(BIN_DIR)/pufu_os

# Reglas principales
all: directories $(NODE) drivers
	@echo "Core Objs: $(CORE_OBJS)"

directories:
	@mkdir -p $(OBJ_DIR)/core
	@mkdir -p $(OBJ_DIR)/vm
	@mkdir -p $(OBJ_DIR)/hal/arm
	@mkdir -p $(OBJ_DIR)/kernel
	@mkdir -p $(BIN_DIR)

# Linkeo - Note: No ARM_OBJS here!
$(NODE): $(CORE_OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lX11 -lEGL -lGLESv2 -lm

# Regla para compilar archivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Ejecutar el nodo bootloader.pufu
run: all drivers
	$(NODE) src/userspace/boot/bootloader.pufu

# Drivers (Shared Objects for Hot Swap)
drivers: directories
	@mkdir -p $(BIN_DIR)/drivers
	$(CC) $(CFLAGS) -fPIC -shared src/hal/arm/arm_socket.c -o $(BIN_DIR)/drivers/socket_arm.so
	$(CC) $(CFLAGS) -fPIC -shared src/hal/arm/arm_socket_net.c -o $(BIN_DIR)/drivers/socket_arm_net.so

.PHONY: all clean run parse directories drivers 
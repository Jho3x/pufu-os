CC = gcc
CFLAGS = -I./include -Wall -Wextra
LDFLAGS = -ldl

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Archivos fuente
CORE_SRCS = $(filter-out $(SRC_DIR)/core/main.c, $(wildcard $(SRC_DIR)/core/*.c))
ARM_SRCS = $(wildcard $(SRC_DIR)/sockets/arm/*.c)
X86_SRCS = $(wildcard $(SRC_DIR)/sockets/x86_64/*.c)
BOOTLOADER = $(SRC_DIR)/core/bootloader.c

# Archivos objeto
CORE_OBJS = $(CORE_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
ARM_OBJS = $(ARM_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
X86_OBJS = $(X86_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BOOTLOADER_OBJ = $(OBJ_DIR)/core/bootloader.o

# Binarios
PARSER = $(BIN_DIR)/pufu_parser
NODE = $(BIN_DIR)/pufu_node

# Reglas principales
all: directories $(PARSER) $(NODE)

directories:
	@mkdir -p $(OBJ_DIR)/core
	@mkdir -p $(OBJ_DIR)/sockets/arm
	@mkdir -p $(OBJ_DIR)/sockets/x86_64
	@mkdir -p $(BIN_DIR)

# Compilar el parser
$(PARSER): $(OBJ_DIR)/core/parser.o $(OBJ_DIR)/core/main.o
	$(CC) $(CFLAGS) -o $@ $^

# Compilar el nodo (usando bootloader.c como main)
$(NODE): $(BOOTLOADER_OBJ) $(CORE_OBJS) $(ARM_OBJS) $(X86_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Regla para compilar archivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Ejecutar el parser con so.pufu
parse: all
	$(PARSER) so.pufu

# Ejecutar el nodo so.pufu
run: all
	$(NODE) so.pufu

.PHONY: all clean run parse directories 
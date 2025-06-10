#include "pufu/socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mensajes de bienvenida
static const char* welcome_messages[] = {
""
};

// Implementación de las funciones del socket ARM
static int arm_init(void) {
    printf("Inicializando socket ARM...\n");
    return 0;
}

static int arm_load_file(const char* filename) {
    printf("Cargando archivo en ARM: %s\n", filename);
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: No se pudo abrir el archivo %s\n", filename);
        return -1;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }
    fclose(file);
    return 0;
}

// Función para manejar syscalls especiales
static int handle_special_syscall(const char* syscall_name, int value) {
    if (strcmp(syscall_name, "(write)") == 0) {
        if (value >= 0 && value < sizeof(welcome_messages)/sizeof(welcome_messages[0])) {
            printf("%s\n", welcome_messages[value]);
            return 0;
        }
    } else if (strcmp(syscall_name, "(exit)") == 0) {
        exit(0);  // Terminar el proceso correctamente
    } else if (strcmp(syscall_name, "(load_file)") == 0) {
        // Cargar el archivo char.pufu
        return arm_load_file("char.pufu");
    } else if (strcmp(syscall_name, "(read_file)") == 0) {
        // Leer el archivo welcome.pufu
        return arm_load_file("welcome.pufu");
    }
    return -1;
}

static int arm_execute(const char* code) {
    // Parsear la instrucción
    char op[32], arg1[32], arg2[32];
    int value;
    
    if (sscanf(code, "%31s %31s %31s", op, arg1, arg2) >= 2) {
        if (strcmp(op, "syscall") == 0) {
            // Es un syscall
            if (arg1[0] == '(' && arg1[strlen(arg1)-1] == ')') {
                // Es un syscall con nombre descriptivo
                return handle_special_syscall(arg1, 0);
            } else {
                // Es un syscall con número
                value = atoi(arg1);
                return handle_special_syscall("(write)", value);
            }
        }
    }
    return 0;
}

static const char* arm_get_arch_name(void) {
    return "ARM";
}

static int arm_get_word_size(void) {
    return 32; // ARM típicamente usa 32 bits
}

static void arm_cleanup(void) {
    printf("Limpiando recursos del socket ARM...\n");
}

// Instancia del socket ARM
static PufuSocket arm_socket = {
    .init = arm_init,
    .execute = arm_execute,
    .load_file = arm_load_file,
    .get_arch_name = arm_get_arch_name,
    .get_word_size = arm_get_word_size,
    .cleanup = arm_cleanup
};

// Función para obtener el socket ARM
PufuSocket* pufu_get_arm_socket(void) {
    return &arm_socket;
}
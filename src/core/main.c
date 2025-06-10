#include "pufu/parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo.pufu>\n", argv[0]);
        return 1;
    }

    // Inicializar el parser
    PufuParser* parser = pufu_parser_init();
    if (!parser) {
        fprintf(stderr, "Error: No se pudo inicializar el parser\n");
        return 1;
    }

    // Parsear el archivo
    if (pufu_parse_file(parser, argv[1]) < 0) {
        fprintf(stderr, "Error: No se pudo parsear el archivo %s\n", argv[1]);
        pufu_parser_cleanup(parser);
        return 1;
    }

    // Imprimir las instrucciones parseadas
    printf("Instrucciones parseadas:\n");
    for (int i = 0; i < parser->count; i++) {
        PufuInstruction* inst = &parser->instructions[i];
        printf("%d: %s", i + 1, inst->op);
        
        if (inst->is_syscall) {
            printf(" %s", inst->value);
        } else {
            printf(" %s", inst->reg1);
            if (inst->reg2[0]) printf(" %s", inst->reg2);
            if (inst->value[0]) printf(" %s", inst->value);
        }
        printf("\n");
    }

    // Liberar recursos
    pufu_parser_cleanup(parser);
    return 0;
} 
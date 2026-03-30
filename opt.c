#include "opt.h"

#include <stdio.h>
#include <string.h>

char *opt_in_file = NULL;
int opt_tokens = 0;
int opt_symtab = 0;
int opt_trace = 0;

static void print_usage(const char *prog) {
    fprintf(stderr, "Uso: %s [--tokens|-k] [--symtab|-s] [--trace|-t] <arquivo.sal>\n", prog);
}

int opts_parse(int argc, char **argv) {
    int i;

    opt_in_file = NULL;
    opt_tokens = 0;
    opt_symtab = 0;
    opt_trace = 0;

    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }

    for (i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "--tokens") == 0 || strcmp(arg, "-k") == 0) {
            opt_tokens = 1;
        } else if (strcmp(arg, "--symtab") == 0 || strcmp(arg, "-s") == 0) {
            opt_symtab = 1;
        } else if (strcmp(arg, "--trace") == 0 || strcmp(arg, "-t") == 0) {
            opt_trace = 1;
        } else if (arg[0] == '-') {
            fprintf(stderr, "Opcao invalida: %s\n", arg);
            print_usage(argv[0]);
            return 0;
        } else if (opt_in_file == NULL) {
            opt_in_file = argv[i];
        } else {
            fprintf(stderr, "Mais de um arquivo de entrada informado: %s\n", arg);
            print_usage(argv[0]);
            return 0;
        }
    }

    if (opt_in_file == NULL) {
        fprintf(stderr, "Arquivo de entrada nao informado.\n");
        print_usage(argv[0]);
        return 0;
    }

    return 1;
}

void opts_get_basename(char *basename_out) {
    const char *base;
    char *dot;

    if (basename_out == NULL) {
        return;
    }

    if (opt_in_file == NULL) {
        basename_out[0] = '\0';
        return;
    }

    base = strrchr(opt_in_file, '/');
    if (base != NULL) {
        base++;
    } else {
        base = opt_in_file;
    }

    strncpy(basename_out, base, 255);
    basename_out[255] = '\0';

    dot = strrchr(basename_out, '.');
    if (dot != NULL) {
        *dot = '\0';
    }
}

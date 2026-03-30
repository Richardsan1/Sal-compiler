#include "diag.h"
#include "opt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *trace_file = NULL;

void diag_init(void) {
    if (opt_trace) {
        char filename[256];
        opts_get_basename(filename);
        strcat(filename, ".trc");
        trace_file = fopen(filename, "w");
        if (!trace_file) {
            fprintf(stderr, "Aviso: Nao foi possivel criar o arquivo de trace %s\n", filename);
        }
    }
}

void diag_error(Token tk, const char* expected_msg) {
    fprintf(stderr, "\n[ERRO SINTATICO] Linha %d:\n", tk.linha);
    fprintf(stderr, "Detalhe: %s\n", expected_msg);
    fprintf(stderr, "Encontrado: '%s' (Categoria: %s)\n", tk.lexema, lex_cat_name(tk.cat));
    fprintf(stderr, "-> Compilacao interrompida.\n\n");
    
    // Fechamento seguro antes de abortar
    if (trace_file) fclose(trace_file);
    exit(1);
}

void diag_info(const char* msg) {
    if (trace_file) {
        fprintf(trace_file, "TRACE: %s\n", msg);
    }
}

void diag_close(void) {
    if (trace_file) {
        fclose(trace_file);
        trace_file = NULL;
    }
}
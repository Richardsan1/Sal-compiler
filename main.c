#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opt.h"
#include "lex.h"
#include "symtab.h"
#include "parser.h"
#include "diag.h"

int main(int argc, char **argv) {
    if (!opts_parse(argc, argv)) {
        return 1;
    }

    FILE *src_file = fopen(opt_in_file, "r");
    if (!src_file) {
        fprintf(stderr, "Erro fatal: Nao foi possivel abrir o arquivo fonte '%s'.\n", opt_in_file);
        return 1;
    }

    char basename[256];
    opts_get_basename(basename);

    // 1. Inicializa os módulos
    lex_init(src_file);
    ts_init();
    diag_init();

    // 2. Dispara a Análise Sintática
    printf("Iniciando compilacao de %s...\n", opt_in_file);
    parse_program();
    printf("Analise lexica e sintatica concluidas com sucesso!\n");

    // 3. Geração de Logs Solicitados
    if (opt_symtab) {
        char ts_filename[300];
        strcpy(ts_filename, basename);
        strcat(ts_filename, ".ts");
        
        FILE *ts_out = fopen(ts_filename, "w");
        if (ts_out) {
            ts_print(ts_out);
            fclose(ts_out);
            printf("Tabela de Simbolos exportada para %s\n", ts_filename);
        } else {
            fprintf(stderr, "Aviso: Falha ao criar arquivo da tabela de simbolos.\n");
        }
    }

    // (Opcional) A geração do log .tk (--tokens) é geralmente injetada 
    // com uma modificação de 3 linhas no lex.c, gravando cada token lido.

    // 4. Encerramento ordenado
    diag_close();
    ts_free();
    fclose(src_file);

    return 0;
}
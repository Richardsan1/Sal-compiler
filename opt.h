#ifndef OPT_H
#define OPT_H

extern char* opt_in_file;
extern int opt_tokens;
extern int opt_symtab;
extern int opt_trace;

// Retorna 1 se o parsing foi bem-sucedido, 0 caso contrário
int opts_parse(int argc, char **argv);

// Retorna o nome base do arquivo (sem extensão) para criar os logs
void opts_get_basename(char *basename_out);

#endif // OPT_H
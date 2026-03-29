#ifndef PARSER_H
#define PARSER_H

#include "lex.h"

// Inicia a análise sintática do programa.
// Exige que o lex_init e ts_init já tenham sido chamados.
void parse_program(void);

#endif // PARSER_H
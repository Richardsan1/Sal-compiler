#ifndef DIAG_H
#define DIAG_H

#include "lex.h"

// Inicializa e finaliza o modulo de diagnostico (trace).
void diag_init(void);
void diag_close(void);

// Reporta um erro de sintaxe/semântica, exibe a linha e aborta a compilação.
void diag_error(Token tk, const char* expected_msg);

// Opcional: Registra informações de rastreamento (flag --trace)
void diag_info(const char* msg);

#endif // DIAG_H
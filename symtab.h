#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>

// Categorias de identificadores
typedef enum {
    symVAR,
    symVEC,
    symFUNC,
    symPROC,
    symPARAM
} SymCategory;

// Tipos de dados da linguagem SAL
typedef enum {
    typeINT,
    typeBOOL,
    typeCHAR,
    typeVOID,
    typeNONE
} SymType;

// Estrutura de um simbolo
typedef struct Symbol {
    char lexema[256];
    SymCategory cat;
    SymType type;
    int extra;  // Tamanho do vetor ou qtd de parametros
    struct Symbol *next;
} Symbol;

// Estrutura de um escopo
typedef struct Scope {
    char name[256];
    Symbol *symbols_head;
    Symbol *symbols_tail;
    struct Scope *parent;
    struct Scope *next_in_log;
} Scope;

// Interface publica da tabela de simbolos
void ts_init(void);

// Gerenciamento de escopos
void ts_push_scope(const char *scope_name);
void ts_pop_scope(void);

// Insercao e busca
// Retorna 1 se sucesso, 0 se o identificador ja existir no escopo atual.
int ts_insert(const char *lexema, SymCategory cat, SymType type, int extra);

// Busca o identificador respeitando a visibilidade (do escopo atual ao global).
Symbol *ts_lookup(const char *lexema);

// Geracao de logs
void ts_print(FILE *out);

// Liberacao de memoria
void ts_free(void);

#endif // SYMTAB_H
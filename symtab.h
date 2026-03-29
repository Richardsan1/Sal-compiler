#ifndef SYMTAB_H
#define SYMTAB_H

#include stdio.h

 Categorias de identificadores
typedef enum {
    symVAR,    Variável simples
    symVEC,    Vetor
    symFUNC,   Função (retorna valor)
    symPROC,   Procedimento (não retorna valor)
    symPARAM   Parâmetro de sub-rotina
} SymCategory;

 Tipos de dados da linguagem SAL
typedef enum {
    typeINT, 
    typeBOOL, 
    typeCHAR, 
    typeVOID,  Para procedimentos
    typeNONE   Para casos onde o tipo não se aplica
} SymType;

 Estrutura de um Símbolo
typedef struct Symbol {
    char lexema[256];
    SymCategory cat;
    SymType type;
    int extra;  Tamanho do vetor ou qtd de parâmetros 
    struct Symbol next;
} Symbol;

 Estrutura de um Escopo
typedef struct Scope {
    char name[256];              Ex global, fnSOMA.locals, procmain.block#1
    Symbol symbols_head;        Início da lista de símbolos deste escopo
    Symbol symbols_tail;        Fim da lista (para inserção rápida e manter ordem)
    struct Scope parent;        Ponteiro para o escopo pai (usado na Pilha de Visibilidade)
    struct Scope next_in_log;   Ponteiro para manter a Lista Mestra (para o log consolidado)
} Scope;

 Interface pública da Tabela de Símbolos
void ts_init(void);

 Gerenciamento de Escopos
void ts_push_scope(const char scope_name);
void ts_pop_scope(void);

 Inserção e Busca
 Retorna 1 se sucesso, 0 se o identificador já existir no escopo atual
int ts_insert(const char lexema, SymCategory cat, SymType type, int extra);

 Busca o identificador respeitando a visibilidade (do escopo atual até o global)
Symbol ts_lookup(const char lexema);

 Geração de Logs
void ts_print(FILE out);

 Liberação de memória
void ts_free(void);

#endif  SYMTAB_H
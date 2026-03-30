#include "symtab.h"
#include <stdlib.h>
#include <string.h>

// Controle da Pilha de Visibilidade (para o Parser usar)
static Scope *active_scope = NULL;

// Controle da Lista Mestra (para gerar o log no final)
static Scope *first_scope = NULL;
static Scope *last_scope = NULL;

void ts_init(void) {
    active_scope = NULL;
    first_scope = NULL;
    last_scope = NULL;
}

void ts_push_scope(const char* scope_name) {
    Scope *new_scope = (Scope*) malloc(sizeof(Scope));
    strncpy(new_scope->name, scope_name, 255);
    new_scope->name[255] = '\0';
    new_scope->symbols_head = NULL;
    new_scope->symbols_tail = NULL;
    
    // Configura a Pilha de Visibilidade
    new_scope->parent = active_scope;
    active_scope = new_scope;
    
    // Configura a Lista Mestra para o log
    new_scope->next_in_log = NULL;
    if (first_scope == NULL) {
        first_scope = new_scope;
        last_scope = new_scope;
    } else {
        last_scope->next_in_log = new_scope;
        last_scope = new_scope;
    }
}

void ts_pop_scope(void) {
    if (active_scope != NULL) {
        active_scope = active_scope->parent;
    }
}

int ts_insert(const char* lexema, SymCategory cat, SymType type, int extra) {
    if (active_scope == NULL) return 0; // Nenhum escopo ativo

    // 1. Verifica se já existe NO ESCOPO ATUAL (evita re-declaração no mesmo bloco)
    Symbol *curr = active_scope->symbols_head;
    while (curr != NULL) {
        if (strcmp(curr->lexema, lexema) == 0) {
            return 0; // Erro: identificador já declarado neste escopo
        }
        curr = curr->next;
    }

    // 2. Aloca e preenche o novo símbolo
    Symbol *new_sym = (Symbol*) malloc(sizeof(Symbol));
    strncpy(new_sym->lexema, lexema, 255);
    new_sym->lexema[255] = '\0';
    new_sym->cat = cat;
    new_sym->type = type;
    new_sym->extra = extra;
    new_sym->next = NULL;

    // 3. Insere no final da lista do escopo atual (preserva ordem de inserção)
    if (active_scope->symbols_head == NULL) {
        active_scope->symbols_head = new_sym;
        active_scope->symbols_tail = new_sym;
    } else {
        active_scope->symbols_tail->next = new_sym;
        active_scope->symbols_tail = new_sym;
    }

    return 1; // Sucesso
}

Symbol* ts_lookup(const char* lexema) {
    // Busca do escopo mais interno (active_scope) até o global (parent = NULL)
    Scope *s = active_scope;
    while (s != NULL) {
        Symbol *sym = s->symbols_head;
        while (sym != NULL) {
            if (strcmp(sym->lexema, lexema) == 0) {
                return sym; // Encontrou a declaração mais próxima/visível
            }
            sym = sym->next;
        }
        s = s->parent; // Sobe um nível de escopo
    }
    return NULL; // Não declarado
}

// Funções auxiliares para traduzir os enums para strings no arquivo de log
static const char* cat_to_str(SymCategory cat) {
    switch(cat) {
        case symVAR: return "VAR";
        case symVEC: return "VEC";
        case symFUNC: return "FUNC";
        case symPROC: return "PROC";
        case symPARAM: return "PARAM";
        default: return "UNKNOWN";
    }
}

static const char* type_to_str(SymType type) {
    switch(type) {
        case typeINT: return "int";
        case typeBOOL: return "bool";
        case typeCHAR: return "char";
        case typeVOID: return "void";
        case typeNONE: return "none";
        default: return "unknown";
    }
}

void ts_print(FILE* out) {
    if (out == NULL) return;

    Scope *s = first_scope;
    while (s != NULL) {
        Symbol *sym = s->symbols_head;
        while (sym != NULL) {
            // Formato exigido pela especificação:
            // SCOPE=<descr>  id="<lexema>"  cat=<categ>  tipo=<tipo>  extra=<atrib>
            fprintf(out, "SCOPE=%-20s id=\"%s\"\tcat=%s\ttipo=%s\textra=%d\n", 
                    s->name, sym->lexema, cat_to_str(sym->cat), type_to_str(sym->type), sym->extra);
            sym = sym->next;
        }
        s = s->next_in_log;
    }
}

void ts_free(void) {
    Scope *s = first_scope;
    while (s != NULL) {
        Scope *next_s = s->next_in_log;
        
        Symbol *sym = s->symbols_head;
        while (sym != NULL) {
            Symbol *next_sym = sym->next;
            free(sym);
            sym = next_sym;
        }
        
        free(s);
        s = next_s;
    }
    ts_init();
}
#include "parser.h"
#include "symtab.h"
#include "diag.h"
#include <stdlib.h>
#include <string.h>

// Lookahead atual
static Token lookahead;
static Token lookahead2;
static int block_counter = 0; // Para nomear escopos anônimos de blocos start..end

// Assinaturas das funções recursivas da gramática
static void match(TokenCat expected);
static void parse_ini(void);
static void parse_glob(void);
static void parse_decls(void);
static void parse_subs(void);
static void parse_func(void);
static void parse_proc(void);
static void parse_princ(void);
static int parse_param(void);
static void parse_bco(void);
static void parse_cmd(void);
static void parse_atr(void);
static void parse_vec_tail(void);
static void parse_match(void);
static void parse_wlist(void);
static void parse_whn(void);
static void parse_othr(void);
static void parse_wcnd(void);
static void parse_witem(void);
static void parse_wint(void);
static void parse_for(void);
static SymType parse_tpo(void);
static void parse_expr(void);
static void parse_exlog(void);
static void parse_exrel(void);
static void parse_exari(void);
static void parse_exarp(void);
static void parse_fact(void);

// Avança para o próximo token
static void next_token() {
    lookahead = lookahead2;
    lookahead2 = lex_next();
}

// Verifica e consome o token esperado. Se falhar, aborta via diag.
static void match(TokenCat expected) {
    if (lookahead.cat == expected) {
        next_token();
    } else {
        char msg[256];
        sprintf(msg, "Esperado token da categoria %s", lex_cat_name(expected));
        diag_error(lookahead, msg);
    }
}

// Ponto de entrada público
void parse_program(void) {
    lookahead = lex_next();
    lookahead2 = lex_next();
    parse_ini();
    if (lookahead.cat != sEOF) {
        diag_error(lookahead, "Esperado Fim de Arquivo (EOF) apos o modulo principal");
    }
}

// ini ::= "sMODULE" id ";" glob? subs? princ
static void parse_ini(void) {
    diag_info("Entering MODULE");
    match(sMODULE);
    
    // O nome do módulo será o escopo global
    ts_push_scope("global"); 
    
    char mod_name[256];
    strcpy(mod_name, lookahead.lexema);
    match(sIDENTIF);
    match(sPONTO_VIRG);

    if (lookahead.cat == sGLOBALS) {
        parse_glob();
    }

    if (lookahead.cat == sFN || lookahead.cat == sPROC) {
        // Como 'main' tambem começa com sPROC, parse_subs precisa diferenciar
        parse_subs();
    } else if (lookahead.cat != sPROC) {
        // Se não for subs nem proc main, tem erro
        diag_error(lookahead, "Esperado 'globals', sub-rotinas ou 'proc main'");
    }

    parse_princ();
    
    ts_pop_scope(); // Fecha o escopo global no fim do arquivo
}

// glob ::= "sGLOBALS" decls+
static void parse_glob(void) {
    diag_info("Entering GLOBALS");
    match(sGLOBALS);
    do {
        parse_decls();
    } while (lookahead.cat == sIDENTIF);
}

// decls ::= id ("," id)* ":" tpo ";"
static void parse_decls(void) {
    char ids[20][256]; // Suporta até 20 variáveis na mesma linha
    int is_vec[20] = {0};
    int vec_size[20] = {0};
    int count = 0;

    strcpy(ids[count++], lookahead.lexema);
    match(sIDENTIF);

    if (lookahead.cat == sABRE_COL) {
        match(sABRE_COL);
        vec_size[count - 1] = atoi(lookahead.lexema);
        match(sCTEINT);
        match(sFECHA_COL);
        is_vec[count - 1] = 1;
    }

    while (lookahead.cat == sVIRGULA) {
        match(sVIRGULA);
        strcpy(ids[count++], lookahead.lexema);
        match(sIDENTIF);

        if (lookahead.cat == sABRE_COL) {
            match(sABRE_COL);
            vec_size[count - 1] = atoi(lookahead.lexema);
            match(sCTEINT);
            match(sFECHA_COL);
            is_vec[count - 1] = 1;
        }
    }

    match(sDOIS_PONTOS);
    SymType type = parse_tpo();

    match(sPONTO_VIRG);

    // Insere todas as variáveis lidas na tabela de símbolos
    for (int i = 0; i < count; i++) {
        if (!ts_insert(ids[i], is_vec[i] ? symVEC : symVAR, type, vec_size[i])) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Variavel '%.200s' ja declarada neste escopo", ids[i]);
            diag_error(lookahead, msg);
        }
    }
}

// tpo ::= "sINT" | "sBOOL" | "sCHAR"
static SymType parse_tpo(void) {
    SymType t = typeNONE;
    if (lookahead.cat == sINT) {
        t = typeINT;
        match(sINT);
    } else if (lookahead.cat == sBOOL) {
        t = typeBOOL;
        match(sBOOL);
    } else if (lookahead.cat == sCHAR) {
        t = typeCHAR;
        match(sCHAR);
    } else {
        diag_error(lookahead, "Tipo invalido (esperado int, bool ou char)");
    }
    return t;
}

// subs ::= (func | proc)+
static void parse_subs(void) {
    while (lookahead.cat == sFN || (lookahead.cat == sPROC && lookahead2.cat != sMAIN)) {
        if (lookahead.cat == sFN) {
            parse_func();
        } else {
            parse_proc();
        }
    }
}

// func ::= "sFN" id "(" param? ")" ":" tpo (locals)? bco
static void parse_func(void) {
    diag_info("Entering FUNCTION");
    match(sFN);
    
    char fn_name[256];
    strcpy(fn_name, lookahead.lexema);
    match(sIDENTIF);
    
    // Registra a função no escopo atual (geralmente global)
    // O tipo de retorno e a qtd de params seriam preenchidos corretamente
    ts_insert(fn_name, symFUNC, typeNONE, 0);
    Symbol *fn_sym = ts_lookup(fn_name);
    
    // Abre o escopo da função
    char scope_name[300];
    sprintf(scope_name, "fn:%s.locals", fn_name);
    ts_push_scope(scope_name);

    match(sABRE_PAR);
    int param_count = 0;
    if (lookahead.cat == sIDENTIF) {
        param_count = parse_param();
    }
    match(sFECHA_PAR);
    
    match(sDOIS_PONTOS);
    SymType ret_type = parse_tpo();

    if (fn_sym != NULL) {
        fn_sym->type = ret_type;
        fn_sym->extra = param_count;
    }

    if (lookahead.cat == sLOCALS) {
        match(sLOCALS);
        do {
            parse_decls();
        } while (lookahead.cat == sIDENTIF);
    }

    parse_bco();
    
    ts_pop_scope(); // Fecha o escopo da função
}

// proc ::= "sPROC" id "(" param? ")" (locals)? bco
static void parse_proc(void) {
    diag_info("Entering PROCEDURE");
    match(sPROC);
    
    char proc_name[256];
    strcpy(proc_name, lookahead.lexema);
    match(sIDENTIF);
    
    ts_insert(proc_name, symPROC, typeVOID, 0); 
    Symbol *proc_sym = ts_lookup(proc_name);

    char scope_name[300];
    sprintf(scope_name, "proc:%s.locals", proc_name);
    ts_push_scope(scope_name);

    match(sABRE_PAR);
    int param_count = 0;
    if (lookahead.cat == sIDENTIF) {
        param_count = parse_param();
    }
    match(sFECHA_PAR);

    if (proc_sym != NULL) {
        proc_sym->extra = param_count;
    }
    
    if (lookahead.cat == sLOCALS) {
        match(sLOCALS);
        do {
            parse_decls();
        } while (lookahead.cat == sIDENTIF);
    }

    parse_bco();
    
    ts_pop_scope();
}

// princ ::= "sPROC" "sMAIN" "(" ")" (locals)? bco
static void parse_princ(void) {
    diag_info("Entering MAIN");
    match(sPROC);
    match(sMAIN);
    
    ts_push_scope("proc:main.locals");

    match(sABRE_PAR);
    match(sFECHA_PAR);

    if (lookahead.cat == sLOCALS) {
        match(sLOCALS);
        do {
            parse_decls();
        } while (lookahead.cat == sIDENTIF);
    }

    parse_bco();
    ts_pop_scope();
}

// param ::= id ":" tpo ("," id ":" tpo)*
static int parse_param(void) {
    int count = 0;
    do {
        char param_name[256];
        strcpy(param_name, lookahead.lexema);
        match(sIDENTIF);
        match(sDOIS_PONTOS);
        SymType type = parse_tpo();
        
        ts_insert(param_name, symPARAM, type, 0);
        count++;
        
        if (lookahead.cat == sVIRGULA) {
            match(sVIRGULA);
        } else {
            break;
        }
    } while (1);

    return count;
}

// bco ::= "sSTART" (cmd ";")* "sEND"
static void parse_bco(void) {
    match(sSTART);
    
    // Abre um escopo anônimo de bloco para variáveis limitadas a este start..end (se a linguagem permitisse, mas ajuda na estrutura)
    block_counter++;
    char block_name[50];
    sprintf(block_name, "block#%d", block_counter);
    ts_push_scope(block_name);

    while (lookahead.cat != sEND && lookahead.cat != sEOF) {
        parse_cmd();
        // Em SAL alguns comandos não exigem ponto-e-vírgula no final se forem de bloco (como if/while), mas a EBNF diz (cmd ";")*
        // Para seguir estritamente o manual, adaptamos se necessário, assumimos que todo comando termina em ; ou é validado internamente
        if (lookahead.cat == sPONTO_VIRG) {
            match(sPONTO_VIRG);
        }
    }
    match(sEND);
    
    ts_pop_scope();
}

// cmd ::= out | inp | if | mat | fr | wh | rpt | atr | call | ret | bco
static void parse_cmd(void) {
    switch (lookahead.cat) {
        case sPRINT:
            match(sPRINT); match(sABRE_PAR); parse_expr();
            while(lookahead.cat == sVIRGULA) { match(sVIRGULA); parse_expr(); }
            match(sFECHA_PAR);
            break;
        case sSCAN:
            match(sSCAN); match(sABRE_PAR); match(sIDENTIF); parse_vec_tail(); match(sFECHA_PAR);
            break;
        case sSTART:
            parse_bco();
            break;
        case sRETURN:
            match(sRETURN); parse_expr();
            break;
        case sIF:
            match(sIF); match(sABRE_PAR); parse_expr(); match(sFECHA_PAR); parse_cmd();
            if (lookahead.cat == sELSE) { match(sELSE); parse_cmd(); }
            break;
        case sMATCH:
            parse_match();
            break;
        case sFOR:
            parse_for();
            break;
        case sLOOP:
            match(sLOOP);
            if (lookahead.cat == sWHILE) {
                match(sWHILE); match(sABRE_PAR); parse_expr(); match(sFECHA_PAR); parse_cmd();
            } else {
                while (lookahead.cat != sUNTIL && lookahead.cat != sEOF) {
                    parse_cmd();
                    if (lookahead.cat == sPONTO_VIRG) {
                        match(sPONTO_VIRG);
                    }
                }
                match(sUNTIL); match(sABRE_PAR); parse_expr(); match(sFECHA_PAR);
            }
            break;
        case sIDENTIF:
            if (lookahead2.cat == sABRE_PAR) {
                // Chamada de procedimento
                match(sIDENTIF);
                match(sABRE_PAR); 
                if (lookahead.cat != sFECHA_PAR) {
                    parse_expr();
                    while(lookahead.cat == sVIRGULA) { match(sVIRGULA); parse_expr(); }
                }
                match(sFECHA_PAR);
            } else {
                parse_atr();
            }
            break;
        default:
            diag_error(lookahead, "Comando nao reconhecido");
            break;
    }
}

// Hierarquia de Expressões (Baseada nas precedências da linguagem)
static void parse_expr(void) {
    parse_exlog();
    while (lookahead.cat == sOR) {
        match(sOR);
        parse_exlog();
    }
}

static void parse_exlog(void) {
    parse_exrel();
    while (lookahead.cat == sAND) {
        match(sAND);
        parse_exrel();
    }
}

static void parse_exrel(void) {
    parse_exari();
    while (lookahead.cat == sMAIOR || lookahead.cat == sMAIORIG || 
           lookahead.cat == sMENOR || lookahead.cat == sMENORIG || 
           lookahead.cat == sIGUAL || lookahead.cat == sDIFERENTE) {
        match(lookahead.cat); // Consome o operador relacional
        parse_exari();
    }
}

static void parse_exari(void) {
    parse_exarp();
    while (lookahead.cat == sSOMA || lookahead.cat == sSUBRAT) {
        match(lookahead.cat);
        parse_exarp();
    }
}

static void parse_exarp(void) {
    parse_fact();
    while (lookahead.cat == sMULT || lookahead.cat == sDIV) {
        match(lookahead.cat);
        parse_fact();
    }
}

static void parse_fact(void) {
    if (lookahead.cat == sIDENTIF) {
        // Pode ser variável simples, vetor ou chamada de função
        match(sIDENTIF);
        if (lookahead.cat == sABRE_COL) { // Vetor
            match(sABRE_COL); parse_expr(); match(sFECHA_COL);
        } else if (lookahead.cat == sABRE_PAR) { // Função
            match(sABRE_PAR);
            if (lookahead.cat != sFECHA_PAR) {
                parse_expr();
                while(lookahead.cat == sVIRGULA) { match(sVIRGULA); parse_expr(); }
            }
            match(sFECHA_PAR);
        }
    } else if (lookahead.cat == sCTEINT || lookahead.cat == sCTECHAR || lookahead.cat == sSTRING || lookahead.cat == sTRUE || lookahead.cat == sFALSE) {
        match(lookahead.cat); // Literais
    } else if (lookahead.cat == sABRE_PAR) {
        match(sABRE_PAR); parse_expr(); match(sFECHA_PAR);
    } else if (lookahead.cat == sNEG || lookahead.cat == sSUBRAT) {
        match(lookahead.cat); parse_fact();
    } else {
        diag_error(lookahead, "Fator de expressao invalido");
    }
}

// atr ::= (id | vec) "sATRIB" elem
static void parse_atr(void) {
    match(sIDENTIF);
    parse_vec_tail();
    match(sATRIB);
    parse_expr();
}

// vec tail ::= "[" (expr) "]" | vazio
static void parse_vec_tail(void) {
    if (lookahead.cat == sABRE_COL) {
        match(sABRE_COL);
        parse_expr();
        match(sFECHA_COL);
    }
}

// mat ::= "sMATCH" "(" expr ")" wlist "sEND"
static void parse_match(void) {
    match(sMATCH);
    match(sABRE_PAR);
    parse_expr();
    match(sFECHA_PAR);
    parse_wlist();
    match(sEND);
}

// wlst ::= whn+ (othr)?
static void parse_wlist(void) {
    parse_whn();
    while (lookahead.cat == sWHEN) {
        parse_whn();
    }
    if (lookahead.cat == sOTHERWISE) {
        parse_othr();
    }
}

// whn ::= "sWHEN" wcnd "sIMPLIC" cmd ";"
static void parse_whn(void) {
    match(sWHEN);
    parse_wcnd();
    match(sIMPLIC);
    parse_cmd();
    match(sPONTO_VIRG);
}

// othr ::= "sOTHERWISE" "sIMPLIC" cmd ";"
static void parse_othr(void) {
    match(sOTHERWISE);
    match(sIMPLIC);
    parse_cmd();
    match(sPONTO_VIRG);
}

// wcnd ::= witem ("," witem )*
static void parse_wcnd(void) {
    parse_witem();
    while (lookahead.cat == sVIRGULA) {
        match(sVIRGULA);
        parse_witem();
    }
}

// witem ::= wint | wrnge
static void parse_witem(void) {
    parse_wint();
    if (lookahead.cat == sPTOPTO) {
        match(sPTOPTO);
        parse_wint();
    }
}

// wint ::= ("sSUBRAT")? "sCTEINT"
static void parse_wint(void) {
    if (lookahead.cat == sSUBRAT) {
        match(sSUBRAT);
    }
    match(sCTEINT);
}

// fr ::= "sFOR" atr "sTO" expr ("sSTEP" (id | "sCTEINT"))? "sDO" cmd
static void parse_for(void) {
    match(sFOR);
    parse_atr();
    match(sTO);
    parse_expr();
    if (lookahead.cat == sSTEP) {
        match(sSTEP);
        if (lookahead.cat == sIDENTIF) {
            match(sIDENTIF);
        } else {
            match(sCTEINT);
        }
    }
    match(sDO);
    parse_cmd();
}
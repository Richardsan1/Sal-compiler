#include "lex.h"
#include <ctype.h>
#include <string.h>
#include "opt.h"
#include "diag.h"




static FILE *src = NULL;
static int linha_atual = 1;

void lex_init(FILE *src_file) {
    src = src_file;
    linha_atual = 1;
}

// Retorna uma string com o nome da categoria (útil para o arquivo .tk)
const char* lex_cat_name(TokenCat cat) {
    switch(cat) {
        case sMODULE: return "sMODULE"; case sGLOBALS: return "sGLOBALS";
        case sLOCALS: return "sLOCALS"; case sINT: return "sINT";
        case sBOOL: return "sBOOL"; case sCHAR: return "sCHAR";
        case sFN: return "sFN"; case sPROC: return "sPROC";
        case sMAIN: return "sMAIN"; case sSTART: return "sSTART";
        case sEND: return "sEND"; case sPRINT: return "sPRINT";
        case sSCAN: return "sSCAN"; case sIF: return "sIF";
        case sELSE: return "sELSE"; case sMATCH: return "sMATCH";
        case sWHEN: return "sWHEN"; case sOTHERWISE: return "sOTHERWISE";
        case sFOR: return "sFOR"; case sSTEP: return "sSTEP";
        case sTO: return "sTO"; case sDO: return "sDO";
        case sLOOP: return "sLOOP"; case sWHILE: return "sWHILE";
        case sUNTIL: return "sUNTIL"; case sRETURN: return "sRETURN";
        case sTRUE: return "sTRUE"; case sFALSE: return "sFALSE";
        case sIDENTIF: return "sIDENTIF"; case sCTEINT: return "sCTEINT";
        case sCTECHAR: return "sCTECHAR"; case sSTRING: return "sSTRING";
        case sATRIB: return "sATRIB"; case sIMPLIC: return "sIMPLIC";
        case sPTOPTO: return "sPTOPTO"; case sSOMA: return "sSOMA";
        case sSUBRAT: return "sSUBRAT"; case sMULT: return "sMULT";
        case sDIV: return "sDIV"; case sMAIOR: return "sMAIOR";
        case sMAIORIG: return "sMAIORIG"; case sMENOR: return "sMENOR";
        case sMENORIG: return "sMENORIG"; case sIGUAL: return "sIGUAL";
        case sDIFERENTE: return "sDIFERENTE"; case sAND: return "sAND";
        case sOR: return "sOR"; case sNEG: return "sNEG";
        case sABRE_PAR: return "sABRE_PAR"; case sFECHA_PAR: return "sFECHA_PAR";
        case sABRE_COL: return "sABRE_COL"; case sFECHA_COL: return "sFECHA_COL";
        case sDOIS_PONTOS: return "sDOIS_PONTOS"; case sPONTO_VIRG: return "sPONTO_VIRG";
        case sVIRGULA: return "sVIRGULA"; case sEOF: return "sEOF";
        default: return "sERRO";
    }
}

// Identifica palavras reservadas
static TokenCat classificar_identificador(const char* lexema) {
    if (strcmp(lexema, "module") == 0) return sMODULE;
    if (strcmp(lexema, "globals") == 0) return sGLOBALS;
    if (strcmp(lexema, "locals") == 0) return sLOCALS;
    if (strcmp(lexema, "int") == 0) return sINT;
    if (strcmp(lexema, "bool") == 0) return sBOOL;
    if (strcmp(lexema, "char") == 0) return sCHAR;
    if (strcmp(lexema, "fn") == 0) return sFN;
    if (strcmp(lexema, "proc") == 0) return sPROC;
    if (strcmp(lexema, "main") == 0) return sMAIN;
    if (strcmp(lexema, "start") == 0) return sSTART;
    if (strcmp(lexema, "end") == 0) return sEND;
    if (strcmp(lexema, "print") == 0) return sPRINT;
    if (strcmp(lexema, "scan") == 0) return sSCAN;
    if (strcmp(lexema, "if") == 0) return sIF;
    if (strcmp(lexema, "else") == 0) return sELSE;
    if (strcmp(lexema, "match") == 0) return sMATCH;
    if (strcmp(lexema, "when") == 0) return sWHEN;
    if (strcmp(lexema, "otherwise") == 0) return sOTHERWISE;
    if (strcmp(lexema, "for") == 0) return sFOR;
    if (strcmp(lexema, "step") == 0) return sSTEP;
    if (strcmp(lexema, "to") == 0) return sTO;
    if (strcmp(lexema, "do") == 0) return sDO;
    if (strcmp(lexema, "loop") == 0) return sLOOP;
    if (strcmp(lexema, "while") == 0) return sWHILE;
    if (strcmp(lexema, "until") == 0) return sUNTIL;
    if (strcmp(lexema, "ret") == 0) return sRETURN; 
    if (strcmp(lexema, "true") == 0) return sTRUE;
    if (strcmp(lexema, "false") == 0) return sFALSE;
    
    // Tratamento especial para o operador lógico OR que é a letra 'v'
    if (strcmp(lexema, "v") == 0) return sOR;

    return sIDENTIF;
}

static void pular_espacos_e_comentarios() {
    int c;
    while ((c = fgetc(src)) != EOF) {
        if (c == '\n') {
            linha_atual++;
        } else if (isspace(c)) {
            continue;
        } else if (c == '@') {
            int next_c = fgetc(src);
            if (next_c == '{') {
                // Comentário de bloco: @{ ... }@
                int prev = 0;
                while ((c = fgetc(src)) != EOF) {
                    if (c == '\n') linha_atual++;
                    if (prev == '}' && c == '@') break;
                    prev = c;
                }
            } else {
                // Comentário de linha: @ ...
                ungetc(next_c, src);
                while ((c = fgetc(src)) != EOF && c != '\n');
                if (c == '\n') linha_atual++;
            }
        } else {
            ungetc(c, src);
            break;
        }
    }
}

Token lex_next(void) {
    Token tk;
    tk.lexema[0] = '\0';
    
    pular_espacos_e_comentarios();
    
    int c = fgetc(src);
    tk.linha = linha_atual; 
    
    if (c == EOF) {
        tk.cat = sEOF;
        strcpy(tk.lexema, "EOF");
        return tk;
    }

    // 1. Identificadores e Palavras Reservadas
    if (isalpha(c) || c == '_') {
        int i = 0;
        do {
            if (i < MAX_LEXEMA - 1) tk.lexema[i++] = c;
            c = fgetc(src);
        } while (isalnum(c) || c == '_');
        ungetc(c, src);
        tk.lexema[i] = '\0';
        tk.cat = classificar_identificador(tk.lexema);
        return tk;
    }

    // 2. Constantes Inteiras
    if (isdigit(c)) {
        int i = 0;
        do {
            if (i < MAX_LEXEMA - 1) tk.lexema[i++] = c;
            c = fgetc(src);
        } while (isdigit(c));
        ungetc(c, src);
        tk.lexema[i] = '\0';
        tk.cat = sCTEINT;
        return tk;
    }

    // 3. Strings ("...")
    if (c == '"') {
        int i = 0;
        tk.lexema[i++] = c;
        while ((c = fgetc(src)) != EOF && c != '"') {
            if (i < MAX_LEXEMA - 1) tk.lexema[i++] = c;
        }
        if (c == '"') tk.lexema[i++] = c; // Inclui a aspa final
        tk.lexema[i] = '\0';
        tk.cat = sSTRING;
        return tk;
    }

    // 4. Caracteres ('c')
    if (c == '\'') {
        int i = 0;
        tk.lexema[i++] = c;
        while ((c = fgetc(src)) != EOF && c != '\'') {
            if (i < MAX_LEXEMA - 1) tk.lexema[i++] = c;
        }
        if (c == '\'') tk.lexema[i++] = c;
        tk.lexema[i] = '\0';
        tk.cat = sCTECHAR;
        return tk;
    }

    // 5. Operadores Simples e Compostos
    tk.lexema[0] = c;
    tk.lexema[1] = '\0';
    
    switch (c) {
        case ':':
            c = fgetc(src);
            if (c == '=') {
                strcpy(tk.lexema, ":=");
                tk.cat = sATRIB;
            } else {
                ungetc(c, src);
                tk.cat = sDOIS_PONTOS;
            }
            break;
        case '=':
            c = fgetc(src);
            if (c == '>') {
                strcpy(tk.lexema, "=>");
                tk.cat = sIMPLIC;
            } else {
                ungetc(c, src);
                tk.cat = sIGUAL;
            }
            break;
        case '<':
            c = fgetc(src);
            if (c == '=') {
                strcpy(tk.lexema, "<=");
                tk.cat = sMENORIG;
            } else if (c == '>') {
                strcpy(tk.lexema, "<>");
                tk.cat = sDIFERENTE;
            } else {
                ungetc(c, src);
                tk.cat = sMENOR;
            }
            break;
        case '>':
            c = fgetc(src);
            if (c == '=') {
                strcpy(tk.lexema, ">=");
                tk.cat = sMAIORIG;
            } else {
                ungetc(c, src);
                tk.cat = sMAIOR;
            }
            break;
        case '.':
            c = fgetc(src);
            if (c == '.') {
                strcpy(tk.lexema, "..");
                tk.cat = sPTOPTO;
            } else {
                ungetc(c, src);
                tk.cat = sERRO; // Em SAL, ponto sozinho não é válido
            }
            break;
        
        // Operadores e delimitadores simples
        case ';': tk.cat = sPONTO_VIRG; break;
        case ',': tk.cat = sVIRGULA; break;
        case '(': tk.cat = sABRE_PAR; break;
        case ')': tk.cat = sFECHA_PAR; break;
        case '[': tk.cat = sABRE_COL; break;
        case ']': tk.cat = sFECHA_COL; break;
        case '+': tk.cat = sSOMA; break;
        case '-': tk.cat = sSUBRAT; break;
        case '*': tk.cat = sMULT; break;
        case '/': tk.cat = sDIV; break;
        case '^': tk.cat = sAND; break;
        case '~': tk.cat = sNEG; break;
        
        default:
            tk.cat = sERRO;
            break;
    }
    
    if (opt_tokens && tk.cat != sEOF && tk.cat != sERRO) {
        static FILE *tk_file = NULL;
        if (!tk_file) {
            char filename[256];
            extern void opts_get_basename(char*);
            opts_get_basename(filename);
            strcat(filename, ".tk");
            tk_file = fopen(filename, "w");
        }
        if (tk_file) {
            // Formato: linha  <CATEGORIA>  "<lexema>"
            fprintf(tk_file, "%-4d <%s> \"%s\"\n", tk.linha, lex_cat_name(tk.cat), tk.lexema);
        }
    }
    
    return tk;
}
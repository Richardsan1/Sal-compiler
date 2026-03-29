#ifndef LEX_H
#define LEX_H

#include <stdio.h>

// Categorias lexicais baseadas na EBNF da SAL
typedef enum {
    sMODULE, sGLOBALS, sINT, sBOOL, sCHAR, 
    sFN, sPROC, sMAIN, sSTART, sEND, sPRINT, sSCAN, 
    sIF, sELSE, sMATCH, sWHEN, sOTHERWISE, 
    sFOR, sSTEP, sLOOP, sWHILE, sUNTIL, sRETURN,

    // Identificadores e Literais
    sIDENTIF, sCTEINT, sCTECHAR, sSTRING,

    // Operadores e Delimitadores
    sATRIB,      // :=
    sIMPLIC,     // =>
    sPTOPTO,     // ..
    sSOMA,       // +
    sSUBRAT,     // -
    sMULT,       // *
    sDIV,        // /
    sMAIOR,      // >
    sMAIORIG,    // >=
    sMENOR,      // <
    sMENORIG,    // <=
    sIGUAL,      // =
    sDIFERENTE,  // <>
    sAND,        // ^
    sOR,         // v
    sNEG,        // ~

    // Pontuação básica (você pode criar categorias para estes também)
    sABRE_PAR,   // (
    sFECHA_PAR,  // )
    sABRE_COL,   // [
    sFECHA_COL,  // ]
    sDOIS_PONTOS,// :
    sPONTO_VIRG, // ;
    sVIRGULA,    // ,

    sEOF,        // Fim de arquivo
    sERRO        // Token inválido
} TokenCat;

// Estrutura do Token
#define MAX_LEXEMA 256

typedef struct {
    TokenCat cat;
    char lexema[MAX_LEXEMA];
    int linha;
} Token;

// Interface pública do módulo Léxico
void lex_init(FILE src_file);
Token lex_next(void);
const char lex_cat_name(TokenCat cat); // Útil para gerar o log --tokens

#endif // LEX_H
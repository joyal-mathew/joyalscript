#pragma once

#include "auxiliary.h"
#include "hashmap.h"
#include "stack.h"

#define MAX_TOKEN_STR_LEN 512

typedef struct __Context__ Context;

typedef enum {
    op_Addition, // NOTE: don't reorder
    op_Subtraction,
    op_Multiplication,
    op_Division,

    op_Assignment,
    op_Reassignment,
    op_OpenParenthesis,
    op_CloseParenthesis,
    op_OpenBrace,
    op_CloseBrace,
    NUM_OPERATORS,
} OperatorType;

typedef enum {
    kw_Print,
    kw_Send,
    kw_If,
    kw_Else,
    kw_While,
} Keyword;

typedef enum {
    tt_Integer,
    tt_Operator,
    tt_Identifier,
    tt_Keyword,
    tt_Eof,
} JyTokenType;

typedef struct {
    Context *context;

    const char *program;
    u64 index;

    u64 line;

    HashMap operator_map;
    HashMap keyword_map;

    Stack idents;

    char token_str[MAX_TOKEN_STR_LEN];
    JyTokenType token_type;
    union {
        u64 integer;
        OperatorType operator_type;
        char *ident;
        Keyword keyword;
    };
} Lexer;

RESULT lexer_init(Lexer *lexer, Context *context);
void lexer_deinit(Lexer *lexer);
RESULT lexer_next(Lexer *lexer);
void token_to_str(Lexer *lexer);
u64 op_to_sstr(OperatorType op);

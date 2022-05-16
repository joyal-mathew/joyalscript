#include "auxiliary.h"
#include "hashmap.h"

#pragma once

#define MAX_TOKEN_STR_LEN 512

typedef struct __Context__ Context;

typedef enum {
    op_Addition,
    op_Subtraction,
    op_Multiplication,
    op_Division,
    op_OpenParenthesis,
    op_CloseParenthesis,
    NUM_OPERATORS,
} OperatorType;

typedef enum {
    tt_Integer,
    tt_Operator,
    tt_Eof,
} JyTokenType;

typedef struct {
    Context *context;

    const char *program;
    u64 index;

    u64 line;

    HashMap operator_map;

    char token_str[MAX_TOKEN_STR_LEN];
    JyTokenType token_type;
    union {
        u64 integer;
        OperatorType operator_type;
    };
} Lexer;

RESULT lexer_init(Lexer *lexer);
void lexer_deinit(Lexer *lexer);
RESULT lexer_next(Lexer *lexer);
void token_to_str(Lexer *lexer);

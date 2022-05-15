#include "auxiliary.h"
#include "hashmap.h"

#pragma once

typedef struct __Context__ Context;

typedef enum {
    op_Addition,
    op_Subtraction,
    op_Multiplication,
    op_Division,
    op_OpenParenthesis,
    op_CloseParenthesis,
} OperatorType;

typedef enum {
    tt_Integer,
    tt_Operator,
    tt_Eof,
} JyTokenType;

typedef struct {
    const char *program;
    u64 index;

    u64 line;


    HashMap operator_map;

    Context *context;

    JyTokenType type;

    union {
        u64 integer;
        OperatorType operator_type;
    };
} Lexer;

WARN_UNUSED
bool lexer_init(Lexer *lexer);

void lexer_deinit(Lexer *lexer);

WARN_UNUSED
bool lexer_next(Lexer *lexer);

void print_token(Lexer *lexer);

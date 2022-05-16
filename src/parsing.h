#include "auxiliary.h"
#include "lexing.h"

#pragma once

typedef struct __Context__ Context;

typedef enum {
    st_Expression,
} StatementType;

typedef enum {
    ex_Integer,
    ex_BinaryOperation,
    ex_UnaryOperation,
} ExpressionType;

typedef struct __Expression__ {
    ExpressionType type;
    u64 line;

    union {
        u64 integer;

        struct {
            OperatorType bin_op;

            struct __Expression__ *lhs;
            struct __Expression__ *rhs;
        };

        struct {
            OperatorType un_op;

            struct __Expression__ *oprand;
        };
    };
} Expression;

typedef struct {
    Context *context;

    StatementType statement_type;
    u64 line;

    u8 precedence_lookup[NUM_OPERATORS];

    union {
        Expression expr;
    };
} Parser;

void parser_init(Parser *parser);
void parser_deinit(Parser *parser);
void parser_stmt_deinit(Parser *parser);
RESULT parser_next(Parser *parser);
void print_statement(Parser *parser);

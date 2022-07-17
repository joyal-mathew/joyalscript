#pragma once

#include "auxiliary.h"
#include "lexing.h"

typedef struct __Context__ Context;
typedef struct __Statement__ Statement;

typedef enum {
    ex_Null,
    ex_Integer,
    ex_Identifier,
    ex_BinaryOperation,
    ex_UnaryOperation,
    ex_Block,
    ex_IfElse,
    ex_Function,
} ExpressionType;

typedef struct __Expression__ {
    ExpressionType type;
    u64 line;

    union {
        u64 integer;
        char *ident;

        struct {
            OperatorType bin_op;

            struct __Expression__ *lhs;
            struct __Expression__ *rhs;
        };

        struct {
            OperatorType un_op;

            struct __Expression__ *oprand;
        };

        struct {
            u64 num_statements;
            Statement *statements;
        };

        struct {
            struct __Expression__ *condition;
            struct __Expression__ *on_true;
            struct __Expression__ *on_false;
        };

        struct {
            char **params;
            u64 num_params;
            struct __Expression__ *body;
        };
    };
} Expression;

typedef enum {
    st_Expression,
    st_Print,
    st_Send,
    st_While,
} StatementType;

typedef struct __Statement__ {
    StatementType type;
    u64 line;

    union {
        Expression expr;

        struct {
            Expression while_condition;
            Expression while_body;
        };
    };
} Statement;

typedef struct {
    Context *context;
    u8 precedence_lookup[NUM_OPERATORS];
    Statement statement;
} Parser;

void parser_init(Parser *parser, Context *context);
void parser_deinit(Parser *parser);
void parser_stmt_deinit(Parser *parser);
RESULT parser_next(Parser *parser);

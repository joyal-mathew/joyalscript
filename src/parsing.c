#include <string.h>
#include "parsing.h"
#include "context.h"

#define MAX_PRECEDENCE 2

RESULT parser_binop(Parser *parser, Expression *expr, u8 precedence);

void parser_init(Parser *parser, Context *context) {
    parser->context = context;

    memset(parser->precedence_lookup, 0, NUM_OPERATORS * sizeof (u8));

    parser->precedence_lookup[op_Addition] = 2; // NOTE: MAX_PRECEDENCE must change if this does
    parser->precedence_lookup[op_Subtraction] = 2;
    parser->precedence_lookup[op_Multiplication] = 1;
    parser->precedence_lookup[op_Division] = 1;
}

void parser_deinit(Parser *parser) {
    memset(parser->precedence_lookup, 0, NUM_OPERATORS * sizeof (u8));
}

RESULT parser_expr(Parser *parser, Expression *expr) {
    return parser_binop(parser, expr, MAX_PRECEDENCE);
}

RESULT parser_term(Parser *parser, Expression *expr) {
    Lexer *lexer = &parser->context->lexer;

    expr->line = lexer->line;

    switch (lexer->token_type) {
        case tt_Eof:
            DISPATCH_ERROR(parser->context, expr->line, "Unexpected EOF");
            return TRUE;
        case tt_Integer:
            expr->type = ex_Integer;
            expr->integer = lexer->integer;

            CHECK(lexer_next(lexer));
            break;
        case tt_Operator:
            switch (lexer->operator_type) {
                case op_Subtraction:
                    expr->type = ex_UnaryOperation;
                    expr->un_op = op_Subtraction;
                    expr->oprand = heap_alloc(1, sizeof (Expression));

                    CHECK(lexer_next(lexer));
                    CHECK(parser_expr(parser, expr->oprand));
                    break;
                case op_OpenParenthesis:
                    CHECK(lexer_next(lexer));
                    CHECK(parser_expr(parser, expr));

                    if (lexer->token_type != tt_Operator || lexer->operator_type != op_CloseParenthesis) {
                        token_to_str(lexer);
                        DISPATCH_ERROR_FMT(parser->context, expr->line, "Expected closing parenthesis, not `%s`", lexer->token_str);
                        return TRUE;
                    }

                    CHECK(lexer_next(lexer));
                    break;
                default:
                    token_to_str(&parser->context->lexer);
                    DISPATCH_ERROR_FMT(parser->context, parser->context->lexer.line, "Undefined unary operation `%s`", parser->context->lexer.token_str);
                    return TRUE;
            }
    }

    return FALSE;
}

RESULT parser_binop(Parser *parser, Expression *expr, u8 precedence) {
    if (precedence == 0) {
        return parser_term(parser, expr);
    }

    CHECK(parser_binop(parser, expr, precedence - 1));

    while (parser->context->lexer.token_type == tt_Operator) {
        u8 this_precedence = parser->precedence_lookup[parser->context->lexer.operator_type];

        if (this_precedence == 0) {
            break;
        }

        if (precedence == this_precedence) {
            Expression *lhs = heap_alloc(1, sizeof (Expression));
            *lhs = *expr;
            // memcpy(lhs, expr, sizeof (Expression));

            expr->type = ex_BinaryOperation;
            expr->line = parser->context->lexer.line;
            expr->bin_op = parser->context->lexer.operator_type;
            expr->lhs = lhs;
            expr->rhs = heap_alloc(1, sizeof (Expression));

            CHECK(lexer_next(&parser->context->lexer));
            CHECK(parser_binop(parser, expr->rhs, precedence - 1));
        }
        else {
            break;
        }
    }

    return FALSE;
}

void tree_deallocate(Expression *expr) {
    switch (expr->type) {
        case ex_BinaryOperation:
            tree_deallocate(expr->lhs);
            tree_deallocate(expr->rhs);
            heap_dealloc(expr->lhs);
            heap_dealloc(expr->rhs);
            break;
        case ex_UnaryOperation:
            tree_deallocate(expr->oprand);
            heap_dealloc(expr->oprand);
            break;
        case ex_Integer:
            break;
    }
}

void parser_stmt_deinit(Parser *parser) {
    switch (parser->statement_type) {
        case st_Expression:
            tree_deallocate(&parser->expr);
            break;
    }
}

RESULT parser_next(Parser *parser) {
    parser->statement_type = st_Expression;
    CHECK(parser_expr(parser, &parser->expr));
    parser->line = parser->expr.line;
    return FALSE;
}

void print_expr(Expression *expr) {
    switch (expr->type) {
        u64 op_sstr;

        case ex_Integer:
            printf("%llu", expr->integer);
            break;
        case ex_BinaryOperation:
            putchar('(');
            op_sstr = op_to_sstr(expr->bin_op);
            printf("%s ", (char *) &op_sstr);
            print_expr(expr->lhs);
            putchar(' ');
            print_expr(expr->rhs);
            putchar(')');

            break;
        case ex_UnaryOperation:
            putchar('(');
            op_sstr = op_to_sstr(expr->un_op);
            printf("%s ", (char *) &op_sstr);
            print_expr(expr->oprand);
            putchar(')');

            break;
    }
}

void print_statement(Parser *parser) {
    switch (parser->statement_type) {
        case st_Expression:
            print_expr(&parser->expr);
            break;
    }
}

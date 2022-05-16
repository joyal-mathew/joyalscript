#include <string.h>
#include "parsing.h"
#include "context.h"

#define MAX_PRECEDENCE 2

RESULT parser_binop(Parser *parser, Expression *expr, u8 precedence);

void parser_init(Parser *parser) {
    memset(parser->precedence_lookup, 0, NUM_OPERATORS * sizeof (u8));

    parser->precedence_lookup[op_Addition] = 2; // NOTE: MAX_PRECEDENCE depends on this
    parser->precedence_lookup[op_Subtraction] = 2;
    parser->precedence_lookup[op_Multiplication] = 1;
    parser->precedence_lookup[op_Division] = 1;
}

void parser_deinit(Parser *parser) {
    memset(parser->precedence_lookup, 0, NUM_OPERATORS * sizeof (u8)); // TODO: remove this
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
                    expr->oprand = allocate(1, sizeof (Expression));

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
            Expression *lhs = allocate(1, sizeof (Expression));
            memcpy(lhs, expr, sizeof (Expression));

            expr->type = ex_BinaryOperation;
            expr->line = parser->context->lexer.line;
            expr->bin_op = parser->context->lexer.operator_type;
            expr->lhs = lhs;
            expr->rhs = allocate(1, sizeof (Expression));

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
            deallocate(expr->lhs);
            deallocate(expr->rhs);
            break;
        case ex_UnaryOperation:
            tree_deallocate(expr->oprand);
            deallocate(expr->oprand);
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
        case ex_Integer:
            printf("%llu", expr->integer);
            break;
        case ex_BinaryOperation:
            putchar('(');
            switch (expr->bin_op) {
                case op_Addition:       printf("+ ");    break;
                case op_Subtraction:    printf("- ");    break;
                case op_Multiplication: printf("* ");    break;
                case op_Division:       printf("/ ");    break;
                default: break;
            }
            print_expr(expr->lhs);
            putchar(' ');
            print_expr(expr->rhs);
            putchar(')');

            break;
        case ex_UnaryOperation:
            putchar('(');
            switch (expr->un_op) {
                case op_Addition:       printf("+ ");    break;
                case op_Subtraction:    printf("- ");    break;
                case op_Multiplication: printf("* ");    break;
                case op_Division:       printf("/ ");    break;
                default: break;
            }
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

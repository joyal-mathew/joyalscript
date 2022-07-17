#include <string.h>
#include "parsing.h"
#include "context.h"

#define MAX_PRECEDENCE 3

RESULT parser_binop(Parser *parser, Expression *expr, u8 precedence);

void parser_init(Parser *parser, Context *context) {
    parser->context = context;

    memset(parser->precedence_lookup, 0, NUM_OPERATORS * sizeof (u8));

    parser->precedence_lookup[op_Assignment] = 3; // NOTE: MAX_PRECEDENCE must change if this does
    parser->precedence_lookup[op_Reassignment] = 3;
    parser->precedence_lookup[op_Addition] = 2;
    parser->precedence_lookup[op_Subtraction] = 2;
    parser->precedence_lookup[op_Multiplication] = 1;
    parser->precedence_lookup[op_Division] = 1;
}

void parser_deinit(UNUSED Parser *parser) {}

bool is_op(Parser *parser, OperatorType op) {
    return parser->context->lexer.token_type == tt_Operator && parser->context->lexer.operator_type == op;
}

bool is_keyword(Parser *parser, Keyword kw) {
    return parser->context->lexer.token_type == tt_Keyword && parser->context->lexer.keyword == kw;
}

RESULT parser_expr(Parser *parser, Expression *expr) {
    CHECK(parser_binop(parser, expr, MAX_PRECEDENCE));
    return FALSE;
}

void print_expr(Expression *expr);

void print_statement(Statement *statement) {
    switch (statement->type) {
    case st_Expression:
        print_expr(&statement->expr);
        break;
    case st_Print:
        printf("PRINT\t");
        print_expr(&statement->expr);
        break;
    case st_Send:
        printf("SEND\t");
        print_expr(&statement->expr);
        break;
    case st_While:
        printf("WHILE\t");
        print_expr(&statement->while_condition);
        print_expr(&statement->while_body);
        break;
    }
}

RESULT paren_expr_or_null(Parser *parser, Expression *expr) {
    if (!is_op(parser, op_OpenParenthesis)) {
        DISPATCH_ERROR(parser->context, parser->context->lexer.line, "Expected open parenthesis");
        return TRUE;
    }

    CHECK(lexer_next(&parser->context->lexer));

    if (is_op(parser, op_CloseParenthesis)) {
        expr->type = ex_Null;
    }
    else {
        CHECK(parser_expr(parser, expr));

        if (!is_op(parser, op_CloseParenthesis)) {
            DISPATCH_ERROR(parser->context, parser->context->lexer.line, "Expected close parenthesis");
            return TRUE;
        }
    }

    return lexer_next(&parser->context->lexer);
}

RESULT parser_while(Parser *parser, Statement *statement) {
    statement->type = st_While;
    statement->line = parser->context->lexer.line;

    CHECK(lexer_next(&parser->context->lexer));
    CHECK(paren_expr_or_null(parser, &statement->while_condition));
    CHECK(parser_expr(parser, &statement->while_body));

    if (statement->while_condition.type == ex_Null) {
        DISPATCH_ERROR(parser->context, parser->context->lexer.line, "Expected non-empty condition in while loop");
        return TRUE;
    }

    return FALSE;
}

RESULT parser_function(Parser *parser, Expression *expr) {
    Stack params;

    expr->type = ex_Function;
    expr->line = parser->context->lexer.line;

    CHECK(lexer_next(&parser->context->lexer));
    stack_init(&params, sizeof (char *));

    while (!is_op(parser, op_Colon)) {
        if (parser->context->lexer.token_type != tt_Identifier) {
            DISPATCH_ERROR(parser->context, parser->context->lexer.line, "Expected identifier in function parameters");
            return TRUE;
        }

        stack_push(&params, &parser->context->lexer.ident);
    }

    expr->params = (char **) params.arr;
    expr->num_params = stack_len(&params);
    CHECK(parser_expr(parser, expr->body));

    return FALSE;
}

RESULT parser_statement(Parser *parser, Statement *statement) {
    if (parser->context->lexer.token_type == tt_Keyword) {
        switch (parser->context->lexer.keyword) {
        case kw_Print:
            statement->type = st_Print;
            CHECK(lexer_next(&parser->context->lexer));
            CHECK(parser_expr(parser, &statement->expr));
            statement->line = statement->expr.line;
            return FALSE;
        case kw_Send:
            statement->type = st_Send;
            CHECK(lexer_next(&parser->context->lexer));
            CHECK(parser_expr(parser, &statement->expr));
            statement->line = statement->expr.line;
            return FALSE;
        case kw_While:
            CHECK(parser_while(parser, statement));
            return FALSE;
        default:
            break;
        }
    }

    statement->type = st_Expression;
    CHECK(parser_expr(parser, &statement->expr));
    statement->line = statement->expr.line;

    return FALSE;
}

RESULT parser_block_general(Parser *parser, Expression *expr) {
    Stack statements;

    stack_init(&statements, sizeof (Statement));

    expr->type = ex_Block;
    expr->line = parser->context->lexer.line;

    while (parser->context->lexer.token_type != tt_Eof && !is_op(parser, op_CloseBrace)) {
        CHECK(parser_statement(parser, stack_reserve(&statements)));
    }

    expr->num_statements = stack_len(&statements);
    expr->statements = (Statement *) statements.arr;

    return FALSE;
}

RESULT parser_block(Parser *parser, Expression *expr) {
    CHECK(lexer_next(&parser->context->lexer));
    CHECK(parser_block_general(parser, expr));

    if (parser->context->lexer.token_type == tt_Eof) {
        DISPATCH_ERROR(parser->context, parser->context->lexer.line, "Unexpected EOF in block");
        return TRUE;
    }

    return lexer_next(&parser->context->lexer);
}

RESULT parser_ifelse(Parser *parser, Expression *expr) {
    expr->condition= heap_alloc(1, sizeof (Expression));
    expr->on_true = heap_alloc(1, sizeof (Expression));
    expr->on_false = NULL;
    expr->type = ex_IfElse;
    expr->line = parser->context->lexer.line;

    CHECK(lexer_next(&parser->context->lexer));
    CHECK(paren_expr_or_null(parser, expr->condition));
    CHECK(parser_expr(parser, expr->on_true));

    if (expr->condition->type == ex_Null) {
        DISPATCH_ERROR(parser->context, parser->context->lexer.line, "Expected non-empty condition in if statement");
        return TRUE;
    }

    if (is_keyword(parser, kw_Else)) {
        expr->on_false = heap_alloc(1, sizeof (Expression));
        CHECK(lexer_next(&parser->context->lexer));
        CHECK(parser_expr(parser, expr->on_false));
    }

    return FALSE;
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
    case tt_Identifier:
        expr->type = ex_Identifier;
        expr->ident = lexer->ident;

        CHECK(lexer_next(lexer));
        break;
    case tt_Keyword:
        switch (lexer->keyword) {
        case kw_If:
            CHECK(parser_ifelse(parser, expr));
            break;
        default:
            token_to_str(lexer);
            DISPATCH_ERROR_FMT(lexer->context, lexer->line, "Unexpected keyword `%s`", lexer->token_str);
            return TRUE;
        }

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

            if (!is_op(parser, op_CloseParenthesis)) {
                token_to_str(lexer);
                DISPATCH_ERROR_FMT(parser->context, expr->line, "Expected closing parenthesis, not `%s`", lexer->token_str);
                return TRUE;
            }

            CHECK(lexer_next(lexer));
            break;
        case op_OpenBrace:
            CHECK(parser_block(parser, expr));
            break;
        case op_Lambda:
            CHECK(parser_function(parser, expr));
            break;
        default:
            token_to_str(&parser->context->lexer);
            DISPATCH_ERROR_FMT(parser->context, parser->context->lexer.line, "Unexpected operator `%s`", parser->context->lexer.token_str);
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

        if (this_precedence && precedence == this_precedence) {
            Expression *lhs = heap_alloc(1, sizeof (Expression));
            *lhs = *expr;

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

void tree_dealloc(Expression *expr);

void statement_dealloc(Statement *statement) {
    switch (statement->type) {
    case st_Expression:
    case st_Print:
    case st_Send:
        tree_dealloc(&statement->expr);
        break;
    case st_While:
        tree_dealloc(&statement->while_condition);
        tree_dealloc(&statement->while_body);
        break;
    }
}

void tree_dealloc(Expression *expr) {
    switch (expr->type) {
    case ex_BinaryOperation:
        tree_dealloc(expr->lhs);
        tree_dealloc(expr->rhs);
        heap_dealloc(expr->lhs);
        heap_dealloc(expr->rhs);
        break;
    case ex_UnaryOperation:
        tree_dealloc(expr->oprand);
        heap_dealloc(expr->oprand);
        break;
    case ex_Block:
        for (u64 i = 0; i < expr->num_statements; ++i) {
            statement_dealloc(expr->statements + i);
        }
        heap_dealloc(expr->statements);
        break;
    case ex_IfElse:
        tree_dealloc(expr->condition);
        tree_dealloc(expr->on_true);
        heap_dealloc(expr->condition);
        heap_dealloc(expr->on_true);

        if (expr->on_false) {
            tree_dealloc(expr->on_false);
            heap_dealloc(expr->on_false);
        }

        break;
    case ex_Function:
        tree_dealloc(expr->body);
        heap_dealloc(expr->body);
        heap_dealloc(expr->params);
        break;
    case ex_Identifier:
    case ex_Integer:
    case ex_Null:
        break;
    }
}

void parser_stmt_deinit(Parser *parser) {
    statement_dealloc(&parser->statement);
}

RESULT parser_next(Parser *parser) {
    parser->statement.type = st_Expression;
    parser->statement.line = parser->statement.expr.line;
    CHECK(parser_block_general(parser, &parser->statement.expr));

    if (parser->context->lexer.token_type != tt_Eof) {
        parser->context->error_line = parser->context->lexer.line;
        strcpy_s(parser->context->error_msg, ERROR_MSG_LEN, "Expected EOF but got }");
        return TRUE;
    }

    return FALSE;
}

void print_expr(Expression *expr) {
    switch (expr->type) {
        u64 op_sstr;

    case ex_Integer:
        printf("%llu", expr->integer);
        break;
    case ex_Null:
        fprintf(stderr, FATAL "Got null exression in print\n");
        exit(-1);
        break;
    case ex_Identifier:
        printf("%s", expr->ident);
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
    case ex_Block:
        putchar('{');
        for (u64 i = 0; i < expr->num_statements; ++i) {
            print_statement(expr->statements + i);
            puts("");
        }
        putchar('}');

        break;
    case ex_IfElse:
        printf("if ");
        print_expr(expr->condition);
        printf("then ");
        print_expr(expr->on_true);

        if (expr->on_false) {
            printf("else ");
            print_expr(expr->on_false);
        }

        break;
    case ex_Function:
        fprintf(stderr, FATAL "Function print");
        exit(-1);
        break;
    }
}

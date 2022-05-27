#include "compiling.h"
#include "parsing.h"
#include "context.h"

#define DEFAULT_BYTECODE_LEN 512

#define DRCT_LABEL      0x00
#define INST_PUSH_INT   0x01
#define INST_ADD        0x02
#define INST_SUB        0x03
#define INST_MUL        0x04
#define INST_DIV        0x05
#define INST_NEG        0x06
#define INST_POP        0x07
#define INST_HALT       0x08

#define OP_OFFSET INST_ADD

void compiler_init(Compiler *compiler, Context *context) {
    compiler->context = context;

    bytevec_init(&compiler->bytecode, DEFAULT_BYTECODE_LEN);
}

void compiler_deinit(Compiler *compiler) {
    bytevec_deinit(&compiler->bytecode);
}

RESULT compile_expr(Compiler *compiler, Expression *expr) {
    switch (expr->type) {
        u64 op_sstr;

        case ex_Integer:
            bytevec_push_byte(&compiler->bytecode, INST_PUSH_INT);
            bytevec_push_qword(&compiler->bytecode, expr->integer);
            break;
        case ex_BinaryOperation:
            CHECK(compile_expr(compiler, expr->lhs));
            CHECK(compile_expr(compiler, expr->rhs));
            bytevec_push_byte(&compiler->bytecode, OP_OFFSET + expr->bin_op);
            break;
        case ex_UnaryOperation:
            CHECK(compile_expr(compiler, expr->oprand));

            switch (expr->un_op) {
                case op_Subtraction:
                    bytevec_push_byte(&compiler->bytecode, INST_SUB);
                    break;
                default:
                    op_sstr = op_to_sstr(expr->un_op);
                    DISPATCH_ERROR_FMT(compiler->context, expr->line, "Invalid unary operator `%s`", (char *) &op_sstr);
                    return TRUE;
            }

            break;
    }

    return FALSE;
}

RESULT compiler_compile(Compiler *compiler) {
    while (compiler->context->lexer.token_type != tt_Eof) {
        CHECK(parser_next(&compiler->context->parser));

        switch (compiler->context->parser.statement_type) {
            case st_Expression:
                CHECK(compile_expr(compiler, &compiler->context->parser.expr));
                bytevec_push_byte(&compiler->bytecode, INST_POP);
        }

        parser_stmt_deinit(&compiler->context->parser);
    }

    bytevec_push_byte(&compiler->bytecode, INST_HALT);

    return FALSE;
}

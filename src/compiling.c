#include "compiling.h"
#include "parsing.h"
#include "context.h"
#include "vm.h"

#define DRCT_LABEL      0x00 // NOTE: inst_names and instructions must change if this does
#define INST_PUSH_INT   0x01
#define INST_PUSH       0x02
#define INST_ADD        0x03
#define INST_SUB        0x04
#define INST_MUL        0x05
#define INST_DIV        0x06
#define INST_NEG        0x07
#define INST_POP        0x08
#define INST_PULL_TO    0x09
#define INST_HALT       0x0A

#define OP_OFFSET INST_ADD

void scope_init(Scope *scope, Scope *parent) {
    hashmap_init(&scope->vars);
    scope->parent = parent;
    scope->ptr = 0;
}

u64 scope_assign(Scope *scope, const char *ident) {
    u64 ptr;

    if (hashmap_get_or_put(&scope->vars, ident, scope->ptr, &ptr)) {
        ++scope->ptr;
    }

    return ptr;
}

RESULT scope_get(Scope *scope, const char *ident, u64 *ptr) {
    return hashmap_get(&scope->vars, ident, ptr);
}

void compiler_init(Compiler *compiler, Context *context) {
    compiler->context = context;
    compiler->scope = heap_alloc(1, sizeof (Scope));

    bytevec_init(&compiler->bytecode);
    scope_init(compiler->scope, NULL);
}

void compiler_deinit(Compiler *compiler) {
    bytevec_deinit(&compiler->bytecode);
    ASSERT(compiler->scope->parent == NULL);
    heap_dealloc(compiler->scope);
}

void compiler_emit_instruction(Compiler *compiler, u8 instruction) {
    #ifdef EBUG_BYTECODE
    printf("> %s\n", inst_names[instruction]);
    #endif

    bytevec_push_byte(&compiler->bytecode, instruction);
}

void compiler_emit_qword(Compiler *compiler, u8 qword) {
    #ifdef EBUG_BYTECODE
    printf("> %llu\n", qword);
    #endif

    bytevec_push_qword(&compiler->bytecode, qword);
}

void compiler_emit_byte(Compiler *compiler, u8 byte) {
    compiler_emit_instruction(compiler, byte);
}

RESULT compile_expr(Compiler *compiler, Expression *expr);

RESULT compile_assignment(Compiler *compiler, Expression *expr) {
    switch (expr->lhs->type) {
        case ex_Identifier:
            CHECK(compile_expr(compiler, expr->rhs));
            compiler_emit_instruction(compiler, INST_PULL_TO);
            compiler_emit_qword(compiler, scope_assign(compiler->scope, expr->lhs->ident));
            break;
        default:
            DISPATCH_ERROR(compiler->context, expr->lhs->line, "Invalid left-hand side of assignment");
            return TRUE;
    }

    return FALSE;
}

RESULT compile_expr(Compiler *compiler, Expression *expr) {
    switch (expr->type) {
        u64 op_sstr;
        u64 ptr;

        case ex_Integer:
            compiler_emit_instruction(compiler, INST_PUSH_INT);
            compiler_emit_qword(compiler, expr->integer);
            break;
        case ex_Identifier:
            if (scope_get(compiler->scope, expr->ident, &ptr)) {
                DISPATCH_ERROR_FMT(compiler->context, expr->line, "Undefined variable `%s`", expr->ident);
                return TRUE;
            }

            compiler_emit_instruction(compiler, INST_PUSH);
            compiler_emit_qword(compiler, ptr);
            break;
        case ex_BinaryOperation:
            if (expr->bin_op == op_Assignment) {
                CHECK(compile_assignment(compiler, expr));
            }
            else {
                CHECK(compile_expr(compiler, expr->lhs));
                CHECK(compile_expr(compiler, expr->rhs));
                compiler_emit_instruction(compiler, OP_OFFSET + expr->bin_op);
            }
            break;
        case ex_UnaryOperation:
            CHECK(compile_expr(compiler, expr->oprand));

            switch (expr->un_op) {
                case op_Subtraction:
                    compiler_emit_instruction(compiler, INST_SUB);
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
                compiler_emit_instruction(compiler, INST_POP);
        }

        parser_stmt_deinit(&compiler->context->parser);
    }

    compiler_emit_instruction(compiler, INST_HALT);

    return FALSE;
}

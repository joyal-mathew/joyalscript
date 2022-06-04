#include "compiling.h"
#include "parsing.h"
#include "context.h"
#include "vm.h"

#define INST_PUSH_INT   0x00 // NOTE: inst_names and instructions must change if this does
#define INST_PUSH_NONE  0x01
#define INST_PUSH       0x02
#define INST_ADD        0x03
#define INST_SUB        0x04
#define INST_MUL        0x05
#define INST_DIV        0x06
#define INST_NEG        0x07
#define INST_POP        0x08
#define INST_PULL_TO    0x09
#define INST_HALT       0x0A
#define INST_SCOPE      0x0B
#define INST_EXIT       0x0C
#define INST_PRINT      0x0D

#define OP_OFFSET INST_ADD

void compiler_scope(Compiler *compiler) {
    Scope *scope = heap_alloc(1, sizeof (Scope));

    hashmap_init(&scope->vars);
    scope->parent = compiler->scope;
    scope->ptr = 0;

    compiler->scope = scope;
}

void compiler_exit(Compiler *compiler) {
    Scope *scope = compiler->scope;
    compiler->scope = compiler->scope->parent;

    hashmap_deinit(&scope->vars);
    heap_dealloc(scope);
}

u64 scope_assign(Scope *scope, const char *ident) {
    u64 ptr;

    if (hashmap_get_or_put(&scope->vars, ident, scope->ptr, &ptr)) {
        ++scope->ptr;
    }

    return ptr;
}

RESULT scope_get(Scope *scope, const char *ident, u64 *ptr, u64 *depth) {
    for (u64 i = 0; scope; scope = scope->parent, ++i) {
        if (!hashmap_get(&scope->vars, ident, ptr)) {
            *depth = i;
            return FALSE;
        }
    }

    return TRUE;
}

void compiler_init(Compiler *compiler, Context *context) {
    compiler->context = context;
    compiler->scope = heap_alloc(1, sizeof (Scope));
    compiler->uid_counter = 0;

    stack_init(&compiler->bytecode, sizeof (u64));
    compiler->scope = NULL;
}

void compiler_deinit(Compiler *compiler) {
    stack_deinit(&compiler->bytecode);

    if (compiler->scope) {
        hashmap_deinit(&compiler->scope->vars);
        heap_dealloc(compiler->scope);
    }
}

void compiler_emit_instruction(Compiler *compiler, u8 instruction) {
#ifdef EBUG_BYTECODE
    printf("> %s\n", inst_names[instruction]);
#endif

    stack_push_byte(&compiler->bytecode, instruction);
}

void compiler_emit_qword(Compiler *compiler, u64 qword) {
#ifdef EBUG_BYTECODE
    printf("> %016X\n", qword);
#endif

    stack_push(&compiler->bytecode, &qword);
}

void compiler_emit_byte(Compiler *compiler, u8 byte) {
    compiler_emit_instruction(compiler, byte);
}

RESULT compile_expr(Compiler *compiler, Expression *expr);

RESULT compile_assignment(Compiler *compiler, Expression *expr, bool reassign) {
    switch (expr->lhs->type) {
        u64 ptr;
        u64 depth;

    case ex_Identifier:
        CHECK(compile_expr(compiler, expr->rhs));
        compiler_emit_instruction(compiler, INST_PULL_TO);

        if (reassign) {
            if (scope_get(compiler->scope, expr->lhs->ident, &ptr, &depth)) {
                DISPATCH_ERROR_FMT(compiler->context, expr->lhs->line, "Variable not already defined `%s`", expr->lhs->ident);
                return TRUE;
            }
        }
        else {
            ptr = scope_assign(compiler->scope, expr->lhs->ident);
            depth = 0;
        }

        compiler_emit_qword(compiler, ptr);
        compiler_emit_qword(compiler, depth);
        break;
    default:
        DISPATCH_ERROR(compiler->context, expr->lhs->line, "Invalid left-hand side of assignment");
        return TRUE;
    }

    return FALSE;
}

RESULT compile_statement(Compiler *compiler, Statement *statement) {
    switch (statement->type) {
    case st_Expression:
        CHECK(compile_expr(compiler, &statement->expr));
        compiler_emit_instruction(compiler, INST_POP);
        break;
    case st_Print:
        CHECK(compile_expr(compiler, &statement->expr));
        compiler_emit_instruction(compiler, INST_PRINT);
        break;
    }

    return FALSE;
}

RESULT compile_expr(Compiler *compiler, Expression *expr) {
    switch (expr->type) {
        u64 op_sstr;
        u64 ptr;
        u64 depth;
        u64 addr;

    case ex_Integer:
        compiler_emit_instruction(compiler, INST_PUSH_INT);
        compiler_emit_qword(compiler, expr->integer);
        break;
    case ex_Identifier:
        if (scope_get(compiler->scope, expr->ident, &ptr, &depth)) {
            DISPATCH_ERROR_FMT(compiler->context, expr->line, "Undefined variable `%s`", expr->ident);
            return TRUE;
        }

        compiler_emit_instruction(compiler, INST_PUSH);
        compiler_emit_qword(compiler, ptr);
        compiler_emit_qword(compiler, depth);
        break;
    case ex_BinaryOperation:
        if (expr->bin_op == op_Assignment) {
            CHECK(compile_assignment(compiler, expr, FALSE));
        }
        else if (expr->bin_op == op_Reassignment) {
            CHECK(compile_assignment(compiler, expr, TRUE));
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
    case ex_Block:
        compiler_emit_instruction(compiler, INST_SCOPE);
        addr = compiler->bytecode.len;
        compiler_emit_qword(compiler, 0);
        compiler_scope(compiler);

        for (u64 i = 0; i < expr->num_statements; ++i) {
            CHECK(compile_statement(compiler, expr->statements + i));
        }

        compiler_emit_instruction(compiler, INST_EXIT);
        compiler_emit_instruction(compiler, INST_PUSH_NONE);
        memcpy(compiler->bytecode.arr + addr, &compiler->scope->ptr, 8);
        compiler_exit(compiler);

        break;
    }

    return FALSE;
}

RESULT compiler_compile(Compiler *compiler) {
    CHECK(parser_next(&compiler->context->parser));
    CHECK(compile_statement(compiler, &compiler->context->parser.statement));
    compiler_emit_instruction(compiler, INST_HALT);

    return FALSE;
}

#include "compiling.h"
#include "parsing.h"
#include "context.h"
#include "vm.h"

#define INST_PUSH_INT   0x00 // NOTE: inst_names, instructions, and NUM_INSTRUCTIONS must change if this does
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
#define INST_JUMP       0x0E
#define INST_BRANCH     0x0F
#define INST_BRANCH_F   0x10

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

    assembler_init(&compiler->assembler);
    compiler->scope = NULL;
}

void compiler_deinit(Compiler *compiler) {
    assembler_deinit(&compiler->assembler);

    if (compiler->scope) {
        hashmap_deinit(&compiler->scope->vars);
        heap_dealloc(compiler->scope);
    }
}

void compiler_emit_instruction(Compiler *compiler, u8 instruction) {
    assembler_emit(&compiler->assembler, (Atom) { at_Byte, { .byte = instruction } });
}

void compiler_emit_byte(Compiler *compiler, u8 byte) {
    assembler_emit(&compiler->assembler, (Atom) { at_Byte, { .byte = byte } });
}

void compiler_emit_qword(Compiler *compiler, u64 number) {
    assembler_emit(&compiler->assembler, (Atom) { at_Number, { .number = number } });
}

void compiler_emit_label_def(Compiler *compiler, u64 label) {
    assembler_emit(&compiler->assembler, (Atom) { at_LabelDef, { .label = label } });
}

void compiler_emit_label_ref(Compiler *compiler, u64 label) {
    assembler_emit(&compiler->assembler, (Atom) { at_LabelRef, { .label = label } });
}

void compiler_emit_var_ref(Compiler *compiler, u64 var) {
    assembler_emit(&compiler->assembler, (Atom) { at_VarRef, { .var_ref = var } });
}

void compiler_emit_var_def(Compiler *compiler, u64 var, u64 val) {
    assembler_emit(&compiler->assembler, (Atom) { at_VarDef, { .var = var, .val = val  } });
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
        u64 loop;
        u64 end;

    case st_Expression:
        CHECK(compile_expr(compiler, &statement->expr));
        compiler_emit_instruction(compiler, INST_POP);
        break;
    case st_Print:
        CHECK(compile_expr(compiler, &statement->expr));
        compiler_emit_instruction(compiler, INST_PRINT);
        break;
    case st_Send:
        CHECK(compile_expr(compiler, &statement->expr));
        break;
    case st_While:
        loop = assembler_get_next(&compiler->assembler);
        end = assembler_get_next(&compiler->assembler);

        compiler_emit_label_def(compiler, loop);
        CHECK(compile_expr(compiler, &statement->while_condition));
        compiler_emit_instruction(compiler, INST_BRANCH_F);
        compiler_emit_label_ref(compiler, end);
        CHECK(compile_expr(compiler, &statement->while_body));
        compiler_emit_instruction(compiler, INST_POP);
        compiler_emit_instruction(compiler, INST_JUMP);
        compiler_emit_label_ref(compiler, loop);
        compiler_emit_label_def(compiler, end);
        break;
    }

    return FALSE;
}

RESULT compile_expr(Compiler *compiler, Expression *expr) {
    switch (expr->type) {
        u64 exit_point;
        u64 scope_size;
        u64 op_sstr;
        u64 ptr;
        u64 depth;
        u64 on_if;
        u64 end;

    case ex_Integer:
        compiler_emit_instruction(compiler, INST_PUSH_INT);
        compiler_emit_qword(compiler, expr->integer);
        break;
    case ex_Null:
        fprintf(stderr, FATAL "Got null expression");
        exit(-1);
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
        exit_point = assembler_get_next(&compiler->assembler);
        scope_size = assembler_get_next(&compiler->assembler);

        compiler_emit_instruction(compiler, INST_SCOPE);
        compiler_emit_var_ref(compiler, scope_size);
        compiler_scope(compiler);

        for (u64 i = 0; i < expr->num_statements; ++i) {
            CHECK(compile_statement(compiler, expr->statements + i));

            switch (expr->statements[i].type) {
            case st_Send:
                compiler_emit_instruction(compiler, INST_JUMP);
                compiler_emit_label_ref(compiler, exit_point);
                break;
            default:
                break;
            }
        }

        compiler_emit_instruction(compiler, INST_PUSH_NONE);
        compiler_emit_label_def(compiler, exit_point);
        compiler_emit_instruction(compiler, INST_EXIT);
        compiler_emit_var_def(compiler, scope_size, compiler->scope->ptr);
        compiler_exit(compiler);
        break;
    case ex_IfElse:
        on_if = assembler_get_next(&compiler->assembler);
        end = assembler_get_next(&compiler->assembler);

        CHECK(compile_expr(compiler, expr->condition));
        compiler_emit_instruction(compiler, INST_BRANCH);
        compiler_emit_label_ref(compiler, on_if);

        if (expr->on_false) CHECK(compile_expr(compiler, expr->on_false));
        else compiler_emit_instruction(compiler, INST_PUSH_NONE);

        compiler_emit_instruction(compiler, INST_JUMP);
        compiler_emit_label_ref(compiler, end);
        compiler_emit_label_def(compiler, on_if);
        CHECK(compile_expr(compiler, expr->on_true));
        compiler_emit_label_def(compiler, end);

        break;
    case ex_Function:
        fprintf(stderr, FATAL "Function compilation");
        exit(-1);
        break;
    }

    return FALSE;
}

RESULT compiler_compile(Compiler *compiler) {
    CHECK(parser_next(&compiler->context->parser));
    CHECK(compile_statement(compiler, &compiler->context->parser.statement));
    compiler_emit_instruction(compiler, INST_HALT);
    assembler_assemble(&compiler->assembler);
    compiler->bytecode = compiler->assembler.bytecode.arr;

    return FALSE;
}

#include <string.h>

#include "vm.h"
#include "context.h"

#define NUM_INSTRUCTIONS 17

const char *type_to_str(ObjectType type) {
    switch (type) {
    case obj_Integer:   return "Integer";
    case obj_None:      return "None";
    default:            return "????";
    }
}

void vm_init(Vm *vm, Context *context) {
    vm->context = context;
    vm->halted = FALSE;
    vm->pc = 0;

    stack_init(&vm->op_stack, sizeof (Object));
    gc_init(&vm->gc);
    vm->scope = NULL;
}

void vm_deinit(Vm *vm) {
    stack_deinit(&vm->op_stack);
    gc_deinit(&vm->gc);
}

RESULT inst_push_int(Vm *vm) {
    u64 integer;

    memcpy(&integer, &vm->program[vm->pc], 8);
    stack_push(&vm->op_stack, &(Object) { obj_Integer, 0, integer });

    vm->pc += 8;

    return FALSE;
}

RESULT inst_push_none(Vm *vm) {
    stack_push(&vm->op_stack, &(Object) { obj_None, 0, 0 });
    return FALSE;
}

RESULT inst_push(Vm *vm) {
    u64 ptr;
    u64 depth;

    memcpy(&ptr, vm->program + vm->pc, 8);
    vm->pc += 8;
    memcpy(&depth, vm->program + vm->pc, 8);
    vm->pc += 8;

    VmScope *scope = vm->scope;

    for (u64 i = 0; i < depth; ++i) {
        scope = scope->parent;

        if (scope == NULL) {
            fprintf(stderr, FATAL "Depth too large\n");
            exit(-1);
        }
    }

    stack_push(&vm->op_stack, scope->stack + ptr);

    return FALSE;
}


RESULT inst_add(Vm *vm) {
    Object rhs;
    stack_pop(&vm->op_stack, &rhs);
    Object *lhs = stack_index(&vm->op_stack, stack_len(&vm->op_stack) - 1);

    i64 rdata = (i64) rhs.data;
    i64 ldata = (i64) lhs->data;

    switch (((u16) lhs->type) << 8 | (u16) rhs.type) {
    case obj_Integer << 8 | obj_Integer:
        lhs->data = (u64) (ldata + rdata);
        break;
    default:
        DISPATCH_ERROR_FMT(vm->context, -1, "Attempt to add invalid types `%s` and `%s`", type_to_str(lhs->type), type_to_str(rhs.type));
        return TRUE;
    }

    return FALSE;
}

RESULT inst_sub(Vm *vm) {
    Object rhs;
    stack_pop(&vm->op_stack, &rhs);
    Object *lhs = stack_index(&vm->op_stack, stack_len(&vm->op_stack) - 1);

    i64 rdata = (i64) rhs.data;
    i64 ldata = (i64) lhs->data;

    switch (((u16) lhs->type) << 8 | (u16) rhs.type) {
    case obj_Integer << 8 | obj_Integer:
        lhs->data = (u64) (ldata - rdata);
        break;
    default:
        DISPATCH_ERROR_FMT(vm->context, -1, "Attempt to subtract invalid types `%s` and `%s`", type_to_str(lhs->type), type_to_str(rhs.type));
        return TRUE;
    }

    return FALSE;
}

RESULT inst_mul(Vm *vm) {
    Object rhs;
    stack_pop(&vm->op_stack, &rhs);
    Object *lhs = stack_index(&vm->op_stack, stack_len(&vm->op_stack) - 1);

    i64 rdata = (i64) rhs.data;
    i64 ldata = (i64) lhs->data;

    switch (((u16) lhs->type) << 8 | (u16) rhs.type) {
    case obj_Integer << 8 | obj_Integer:
        lhs->data = (u64) (ldata * rdata);
        break;
    default:
        DISPATCH_ERROR_FMT(vm->context, -1, "Attempt to multiply invalid types `%s` and `%s`", type_to_str(lhs->type), type_to_str(rhs.type));
        return TRUE;
    }

    return FALSE;
}

RESULT inst_div(Vm *vm) {
    Object rhs;
    stack_pop(&vm->op_stack, &rhs);
    Object *lhs = stack_index(&vm->op_stack, stack_len(&vm->op_stack) - 1);

    i64 rdata = (i64) rhs.data;
    i64 ldata = (i64) lhs->data;

    switch (((u16) lhs->type) << 8 | (u16) rhs.type) {
    case obj_Integer << 8 | obj_Integer:
        lhs->data = (u64) (ldata / rdata);
        break;
    default:
        DISPATCH_ERROR_FMT(vm->context, -1, "Attempt to divide invalid types `%s` and `%s`", type_to_str(lhs->type), type_to_str(rhs.type));
        return TRUE;
    }

    return FALSE;
}

RESULT inst_neg(Vm *vm) {
    Object *oprand = stack_index(&vm->op_stack, stack_len(&vm->op_stack) - 1);

    switch (oprand->type) {
    case obj_Integer:
        oprand->data = ~oprand->data + 1;
        break;
    default:
        DISPATCH_ERROR_FMT(vm->context, -1, "Attempt to negate an invalid type `%s`", type_to_str(oprand->type));
        return TRUE;
    }

    return FALSE;
}

RESULT inst_pop(Vm *vm) {
    if (stack_len(&vm->op_stack) == 0) {
        fprintf(stderr, FATAL "Stack underflow\n");
        exit(-1);
    }

    vm->op_stack.len -= sizeof (Object);

    return FALSE;
}

RESULT inst_pull_to(Vm *vm) {
    u64 ptr;
    u64 depth;

    memcpy(&ptr, vm->program + vm->pc, 8);
    vm->pc += 8;
    memcpy(&depth, vm->program + vm->pc, 8);
    vm->pc += 8;

    VmScope *scope = vm->scope;

    for (u64 i = 0; i < depth; ++i) {
        scope = scope->parent;

        if (scope == NULL) {
            fprintf(stderr, FATAL "Depth too large\n");
            exit(-1);
        }
    }

    memcpy(scope->stack + ptr, stack_index(&vm->op_stack, stack_len(&vm->op_stack) - 1), sizeof (Object));

    return FALSE;
}

RESULT inst_halt(Vm *vm) {
    vm->halted = TRUE;
    return FALSE;
}

RESULT inst_scope(Vm *vm) {
    u64 size;
    memcpy(&size, &vm->program[vm->pc], 8);
    vm->pc += 8;

    Object *scope_obj = gc_alloc(&vm->gc);
    VmScope *scope = heap_alloc(1, sizeof (VmScope));

    scope_obj->type = obj_Scope;
    scope_obj->data = (u64) scope;

    scope->parent = vm->scope;
    scope->stack = heap_alloc(size, sizeof (Object));
    vm->scope = scope;

    return FALSE;
}

RESULT inst_exit(Vm *vm) {
    vm->scope = vm->scope->parent;
    return FALSE;
}

RESULT inst_print(Vm *vm) {
    Object obj;
    stack_pop(&vm->op_stack, &obj);

    switch (obj.type) {
    case obj_Integer:
        printf("%lld\n", obj.data);
        break;
    case obj_None:
        printf("none\n");
        break;
    case obj_Scope:
        fprintf(stderr, "Attempt to print scope");
        exit(-1);
    }

    return FALSE;
}

RESULT inst_jump(Vm *vm) {
    u64 addr;
    memcpy(&addr, vm->program + vm->pc, 8);
    vm->pc = addr;
    return FALSE;
}

RESULT inst_branch(Vm *vm) {
    u64 addr;
    Object condition;
    bool branch;

    stack_pop(&vm->op_stack, &condition);
    memcpy(&addr, vm->program + vm->pc, 8);

    switch (condition.type) {
    case obj_Integer:
    case obj_None:
        branch = condition.data;
        break;
    default:
        DISPATCH_ERROR_FMT(vm->context, -1, "Cannot determine truth value of object with type `%s`", type_to_str(condition.type));
        return TRUE;
    }

    vm->pc = branch ? addr : vm->pc + 8;

    return FALSE;
}

RESULT inst_branch_f(Vm *vm) {
    u64 addr;
    Object condition;
    bool nobranch;

    stack_pop(&vm->op_stack, &condition);
    memcpy(&addr, vm->program + vm->pc, 8);

    switch (condition.type) {
    case obj_Integer:
    case obj_None:
        nobranch = condition.data;
        break;
    default:
        DISPATCH_ERROR_FMT(vm->context, -1, "Cannot determine truth value of object with type `%s`", type_to_str(condition.type));
        return TRUE;
    }

    vm->pc = nobranch ? vm->pc + 8 : addr;

    return FALSE;
}

bool (*instructions[NUM_INSTRUCTIONS]) (Vm *vm) = {
    inst_push_int,
    inst_push_none,
    inst_push,
    inst_add,
    inst_sub,
    inst_mul,
    inst_div,
    inst_neg,
    inst_pop,
    inst_pull_to,
    inst_halt,
    inst_scope,
    inst_exit,
    inst_print,
    inst_jump,
    inst_branch,
    inst_branch_f,
};

const char *inst_names[NUM_INSTRUCTIONS] = {
    "push_int",
    "push_none",
    "push",
    "add",
    "sub",
    "mul",
    "div",
    "neg",
    "pop",
    "pull_to",
    "halt",
    "scope",
    "exit",
    "print",
    "jump",
    "branch",
    "branch_f",
};

RESULT vm_run(Vm *vm) {
    while (!vm->halted) {
        u8 opcode = vm->program[vm->pc++];

#ifdef EBUG_EXE
        for (u64 i = 0; i < 4; ++i) {
            printf("%llu ", ((Object *) stack_index(&vm->op_stack, i))->data);
        }
        printf("\n[x] %hhu\t", opcode);
        printf("%s\n", inst_names[opcode]);
        getchar();
#endif

        CHECK(instructions[opcode](vm));
    }

    return FALSE;
}

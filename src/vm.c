#include <string.h>

#include "vm.h"
#include "context.h"

#define DEFAULT_OPSTACK_SIZE 512
#define NUM_INSTRUCTIONS 9

void vm_init(Vm *vm, Context *context) {
    vm->context = context;
    vm->op_stack = heap_alloc(DEFAULT_OPSTACK_SIZE, sizeof (Object));
    vm->op_stack_len = 0;
    vm->op_stack_cap = DEFAULT_OPSTACK_SIZE;
    vm->halted = FALSE;
    vm->pc = 0;
}

void vm_deinit(Vm *vm) {
    heap_dealloc(vm->op_stack);
}

void vm_opstack_grow(Vm *vm) {
    if (vm->op_stack_len >= vm->op_stack_cap) {
        vm->op_stack_cap = 2 * vm->op_stack_cap + 1;
        vm->op_stack = heap_realloc(vm->op_stack, vm->op_stack_cap, sizeof (Object));
    }
}

void vm_opstack_pop(Vm *vm, Object *obj) {
    if (vm->op_stack_len == 0) {
        fprintf(stderr, FATAL "Stack underflow\n");
        exit(-1);
    }

    *obj = vm->op_stack[--vm->op_stack_len];
}

RESULT inst_inv(UNUSED Vm *vm) {
    fprintf(stderr, FATAL "Invalid instruction");
    exit(-1);
}

RESULT inst_push_int(Vm *vm) {
    vm_opstack_grow(vm);

    vm->op_stack[vm->op_stack_len].type = obj_Integer;
    memcpy(&vm->op_stack[vm->op_stack_len].data, &vm->program[vm->pc], 8);

    vm->op_stack_len++;
    vm->pc += 8;

    return FALSE;
}

RESULT inst_add(Vm *vm) {
    Object rhs;
    vm_opstack_pop(vm, &rhs);
    Object *lhs = &vm->op_stack[vm->op_stack_len - 1];

    i64 rdata = (i64) rhs.data;
    i64 ldata = (i64) lhs->data;

    switch (((u16) rhs.type) << 8 | (u16) lhs->type) {
        case obj_Integer << 8 | obj_Integer:
            lhs->data = (u64) (ldata + rdata);
            break;
    }

    return FALSE;
}

RESULT inst_sub(Vm *vm) {
    Object rhs;
    vm_opstack_pop(vm, &rhs);
    Object *lhs = &vm->op_stack[vm->op_stack_len - 1];

    i64 rdata = (i64) rhs.data;
    i64 ldata = (i64) lhs->data;

    switch (((u16) rhs.type) << 8 | (u16) lhs->type) {
        case obj_Integer << 8 | obj_Integer:
            lhs->data = (u64) (ldata - rdata);
            break;
    }

    return FALSE;
}

RESULT inst_mul(Vm *vm) {
    Object rhs;
    vm_opstack_pop(vm, &rhs);
    Object *lhs = &vm->op_stack[vm->op_stack_len - 1];

    i64 rdata = (i64) rhs.data;
    i64 ldata = (i64) lhs->data;

    switch (((u16) rhs.type) << 8 | (u16) lhs->type) {
        case obj_Integer << 8 | obj_Integer:
            lhs->data = (u64) (ldata * rdata);
            break;
    }

    return FALSE;
}

RESULT inst_div(Vm *vm) {
    Object rhs;
    vm_opstack_pop(vm, &rhs);
    Object *lhs = &vm->op_stack[vm->op_stack_len - 1];

    i64 rdata = (i64) rhs.data;
    i64 ldata = (i64) lhs->data;

    switch (((u16) rhs.type) << 8 | (u16) lhs->type) {
        case obj_Integer << 8 | obj_Integer:
            lhs->data = (u64) (ldata / rdata);
            break;
    }

    return FALSE;
}

RESULT inst_neg(Vm *vm) {
    Object *oprand = &vm->op_stack[vm->op_stack_len - 1];

    switch (oprand->type) {
        case obj_Integer:
            oprand->data = ~oprand->data + 1;
            break;
    }

    return FALSE;
}

RESULT inst_pop(Vm *vm) {
    if (vm->op_stack_len == 0) {
        fprintf(stderr, FATAL "Stack underflow\n");
        exit(-1);
    }

    vm->op_stack_len--;

    return FALSE;
}

RESULT inst_halt(Vm *vm) {
    vm->halted = TRUE;
    return FALSE;
}

void print_obj(Object *obj) {
    switch (obj->type) {
        case obj_Integer:
            printf("%lld", obj->data);
            break;
    }
}

bool (*instructions[NUM_INSTRUCTIONS]) (Vm *vm) = {
    inst_inv,
    inst_push_int,
    inst_add,
    inst_sub,
    inst_mul,
    inst_div,
    inst_neg,
    inst_pop,
    inst_halt,
};

const char *inst_names[NUM_INSTRUCTIONS] = {
    "???",
    "push_int",
    "add",
    "sub",
    "mul",
    "div",
    "neg",
    "pop",
    "halt",
};

RESULT vm_run(Vm *vm) {
    while (!vm->halted) {
        u8 opcode = vm->program[vm->pc++];
        CHECK(instructions[opcode](vm));
    }

    return FALSE;
}

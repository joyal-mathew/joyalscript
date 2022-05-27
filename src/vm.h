#pragma once

#include "auxiliary.h"

typedef struct __Context__ Context;

typedef enum PACKED {
    obj_Integer
} ObjectType;

typedef struct {
    ObjectType type;
    u64 data;
} Object;

_Static_assert(sizeof (ObjectType) == 1, "ObjectType size");
_Static_assert(sizeof (Object) == 16, "Object size");

typedef struct {
    Context *context;

    Object *op_stack;
    u64 op_stack_len;
    u64 op_stack_cap;

    bool halted;

    u8 *program;
    u64 pc;
} Vm;

void vm_init(Vm *vm, Context *context);
void vm_deinit(Vm *vm);
RESULT vm_run(Vm *vm);
void print_obj(Object *obj);

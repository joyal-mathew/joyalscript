#pragma once

#include "auxiliary.h"
#include "compiling.h"
#include "gc.h"

typedef struct __Context__ Context;

typedef enum PACKED {
    obj_Integer,
    obj_None,

    obj_Scope,
} ObjectType;

typedef struct __Object__ {
    ObjectType type;
    u8 mark;
    u64 data;
} Object;

typedef struct __VmScope__ {
    Object *stack;
    struct __VmScope__ *parent;
} VmScope;

_Static_assert(sizeof (ObjectType) == 1, "ObjectType size");
_Static_assert(sizeof (Object) == 16, "Object size");

typedef struct {
    Context *context;
    Stack op_stack;
    VmScope *scope;
    Gc gc;

    bool halted;
    u8 *program;
    u64 pc;
} Vm;

void vm_init(Vm *vm, Context *context);
void vm_deinit(Vm *vm);
RESULT vm_run(Vm *vm);

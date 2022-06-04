#pragma once

#include "auxiliary.h"
#include "stack.h"

typedef struct __Object__ Object;

typedef struct {
    Stack allocations;
    u8 mark;
} Gc;

void gc_init(Gc *gc);
void gc_deinit(Gc *gc);
Object *gc_alloc(Gc *gc);
void gc_collect(Gc *gc, Object *base);

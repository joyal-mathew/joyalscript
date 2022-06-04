#include "gc.h"
#include "vm.h"

void dealloc(Object *obj) {
    switch (obj->type) {
    default:
        break;
    }
}

void gc_init(Gc *gc) {
    stack_init(&gc->allocations, sizeof (Object));
    gc->mark = 0;
}

void gc_deinit(Gc *gc) {
    for (u64 i = 0; i < stack_len(&gc->allocations); ++i) {
        Object *obj = stack_index(&gc->allocations, i);
        dealloc(obj);
    }

    stack_deinit(&gc->allocations);
}

Object *gc_alloc(Gc *gc) {
    Object *ptr = heap_alloc(1, sizeof (Object));
    ptr->mark = gc->mark;
    stack_push(&gc->allocations, &ptr);
    return ptr;
}

void gc_mark(Gc *gc, Object *obj) {
    obj->mark = gc->mark;

    switch (obj->type) {
    default:
        break;
    }
}

void gc_collect(Gc *gc, Object *base) {
    gc->mark = !gc->mark;
    gc_mark(gc, base);

    for (u64 i = 0; i < stack_len(&gc->allocations); ++i) {
        Object *obj = stack_index(&gc->allocations, i);

        if (obj->mark != gc->mark) {
            dealloc(obj);
        }
    }
}

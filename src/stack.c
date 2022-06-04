#include <string.h>
#include "stack.h"

#define DEFAULT_STACK_CAP 512

void stack_init(Stack *stack, u64 elem_size) {
    stack->cap = DEFAULT_STACK_CAP;
    stack->len = 0;
    stack->elem_size = elem_size;
    stack->arr = heap_alloc(DEFAULT_STACK_CAP, sizeof (u8));
}

void stack_deinit(Stack *stack) {
    heap_dealloc(stack->arr);
}

void stack_push(Stack *stack, void *obj) {
    if (stack->len + stack->elem_size > stack->cap) {
        stack->cap = stack->cap * 2 + stack->elem_size;
        stack->arr = heap_realloc(stack->arr, stack->cap, sizeof (u8));
    }

    memcpy(stack->arr + stack->len, obj, stack->elem_size);
    stack->len += stack->elem_size;
}

void stack_pop(Stack *stack, void *obj) {
    if (stack->len < stack->elem_size) {
        fprintf(stderr, FATAL "Stack underflow");
        exit(-1);
    }

    stack->len -= stack->elem_size;
    memcpy(obj, stack->arr + stack->len, stack->elem_size);
}

void stack_push_byte(Stack *stack, u8 byte) {
    if (stack->len + 1 > stack->cap) {
        stack->cap = stack->cap * 2 + 1;
        stack->arr = heap_realloc(stack->arr, stack->cap, sizeof (u8));
    }

    stack->arr[stack->len++] = byte;
}

u8 stack_pop_byte(Stack *stack) {
    if (stack->len < 1) {
        fprintf(stderr, FATAL "Stack underflow");
        exit(-1);
    }

    return stack->arr[--stack->len];
}

void *stack_index(Stack *stack, u64 index) {
    return stack->arr + index * stack->elem_size;
}

void *stack_reserve(Stack *stack) {
    if (stack->len + stack->elem_size > stack->cap) {
        stack->cap = stack->cap * 2 + stack->elem_size;
        stack->arr = heap_realloc(stack->arr, stack->cap, sizeof (u8));
    }

    void *ptr = stack->arr + stack->len;
    stack->len += stack->elem_size;
    return ptr;
}

u64 stack_len(Stack *stack) {
    return stack->len / stack->elem_size;
}

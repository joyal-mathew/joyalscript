#pragma once

#include "auxiliary.h"

typedef struct {
    u8 *arr;
    u64 len;
    u64 cap;
    u64 elem_size;
} Stack;

void stack_init(Stack *stack, u64 elem_size);
void stack_deinit(Stack *stack);
void stack_push(Stack *stack, void *obj);
void stack_pop(Stack *stack, void *obj);
void stack_push_byte(Stack *stack, u8 byte);
void *stack_index(Stack *stack, u64 index);
void *stack_reserve(Stack *stack);
u8 stack_pop_byte(Stack *stack);
u64 stack_len(Stack *stack);

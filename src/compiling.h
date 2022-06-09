#pragma once

#include "auxiliary.h"
#include "stack.h"
#include "hashmap.h"

typedef struct __Context__ Context;

typedef struct __Scope__ {
    HashMap vars;
    u64 ptr;

    struct __Scope__ *parent;
} Scope;

typedef struct {
    Context *context;
    Scope *scope;
    Stack bytecode;

    u64 uid_counter;
} Compiler;

void compiler_init(Compiler *compiler, Context *context);
void compiler_deinit(Compiler *compiler);
RESULT compiler_compile(Compiler *compiler);

#pragma once

#include "auxiliary.h"
#include "bytevec.h"
#include "hashmap.h"

#define NUM_INSTRUCTIONS 11

typedef struct __Context__ Context;

typedef struct __Scope__ {
    HashMap vars;
    u64 ptr;

    struct __Scope__ *parent;
} Scope;

typedef struct {
    Context *context;
    Scope *scope;
    ByteVec bytecode;
} Compiler;

static const char *inst_names[NUM_INSTRUCTIONS] = {
    "???",
    "push_int",
    "push",
    "add",
    "sub",
    "mul",
    "div",
    "neg",
    "pop",
    "pull_to",
    "halt",
};

void compiler_init(Compiler *compiler, Context *context);
void compiler_deinit(Compiler *compiler);
RESULT compiler_compile(Compiler *compiler);

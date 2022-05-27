#pragma once

#include "auxiliary.h"
#include "bytevec.h"

typedef struct __Context__ Context;

typedef struct {
    Context *context;

    ByteVec bytecode;
} Compiler;

void compiler_init(Compiler *compiler, Context *context);
void compiler_deinit(Compiler *compiler);
RESULT compiler_compile(Compiler *compiler);

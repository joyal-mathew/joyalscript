#pragma once

#include "lexing.h"
#include "parsing.h"
#include "compiling.h"
#include "vm.h"

#define ERROR_MSG_LEN 512

#define DISPATCH_ERROR_FMT(context, line, format, ...) do { context->error_line = line; sprintf_s(context->error_msg, ERROR_MSG_LEN, format, __VA_ARGS__); } while (FALSE)
#define DISPATCH_ERROR(context, line, str) do { context->error_line = line; strcpy_s(context->error_msg, ERROR_MSG_LEN, str); } while (FALSE)

typedef struct __Context__ {
    Lexer lexer;
    Parser parser;
    Compiler compiler;
    Vm vm;

    char *program;

    u64 error_line;
    char error_msg[ERROR_MSG_LEN];
} Context;

void context_init(Context *context, const char *path);
void context_deinit(Context *context);
void context_run(Context *context);

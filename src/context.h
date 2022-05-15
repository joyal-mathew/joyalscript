#include "lexing.h"

#pragma once

#define ERROR_MSG_LEN 512

typedef struct __Context__ {
    Lexer lexer;

    char *program;

    u64 error_line;
    char error_msg[ERROR_MSG_LEN];
} Context;

WARN_UNUSED
bool dispatch_error(Context *context, const char *msg, u64 line);

void context_init(Context *context, const char *path);
void context_deinit(Context *context);
void context_run(Context *context);

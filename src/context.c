#include <string.h>
#include <stdio.h>
#include "context.h"

void handle_error(Context *context, bool error) {
    if (error) {
        context_deinit(context);
        fprintf(stderr, ERR "Line %llu: %s\n", context->error_line, context->error_msg);
        exit(1);
    }
}

bool dispatch_error(Context *context, const char *msg, u64 line) {
    strcpy_s(context->error_msg, ERROR_MSG_LEN, msg);
    context->error_line = line;
    return TRUE;
}

void context_init(Context *context, const char *path) {
    context->program = read_file(path);
    context->lexer.context = context;
    handle_error(context, lexer_init(&context->lexer));
}

void context_deinit(Context *context) {
    lexer_deinit(&context->lexer);
    deallocate(context->program);
}

void context_run(Context *context) {
    for (;;) {
        print_token(&context->lexer);

        if (context->lexer.type == tt_Eof) {
            break;
        }

        handle_error(context, lexer_next(&context->lexer));
    }
}

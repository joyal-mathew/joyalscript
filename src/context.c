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

void context_init(Context *context, const char *path) {
    context->program = read_file(path);
    context->lexer.context = context;
    context->parser.context = context;
    parser_init(&context->parser);
    handle_error(context, lexer_init(&context->lexer));
}

void context_deinit(Context *context) {
    lexer_deinit(&context->lexer);
    parser_deinit(&context->parser);
    deallocate(context->program);
}

void context_run(Context *context) {
    while (context->lexer.token_type != tt_Eof) {
        handle_error(context, parser_next(&context->parser));
        print_statement(&context->parser);
        parser_stmt_deinit(&context->parser);
    }
}

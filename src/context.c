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
    parser_init(&context->parser, context);
    compiler_init(&context->compiler, context);
    vm_init(&context->vm, context);
    handle_error(context, lexer_init(&context->lexer, context));
}

void context_deinit(Context *context) {
    lexer_deinit(&context->lexer);
    parser_deinit(&context->parser);
    compiler_deinit(&context->compiler);
    vm_deinit(&context->vm);
    heap_dealloc(context->program);
}

void context_run(Context *context) {
    handle_error(context, compiler_compile(&context->compiler));
    context->vm.program = context->compiler.bytecode;

    handle_error(context, vm_run(&context->vm));
    ASSERT(context->vm.op_stack.len == 0);
}

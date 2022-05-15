#include "context.h"

int main(int argc, char **argv) {
    Context context;

    if (argc < 2) {
        fprintf(stderr, FATAL "File not specified\n");
        return 1;
    }

    context_init(&context, argv[1]);
    context_run(&context);
    context_deinit(&context);
}

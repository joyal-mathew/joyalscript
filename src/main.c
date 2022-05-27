#include "context.h"

// TODO: fixed signedness issue (negation can overflow and literals can be too large to be signed)
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

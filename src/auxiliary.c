#include "auxiliary.h"

void *check_ptr(void *ptr) {
    if (ptr) return ptr;
    fprintf(stderr, FATAL "Out of memory\n");
    exit(-1);
}

void *allocate(u64 count, u64 size) {
    return check_ptr(malloc(count * size));
}

void *reallocate(void *ptr, u64 count, u64 size) {
    return check_ptr(realloc(ptr, count * size));
}

void deallocate(void *ptr) {
    free(ptr);
}

char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, FATAL "Cannot open file\n");
        exit(-1);
    }

    ASSERT(fseek(file, 0, SEEK_END) == 0);

    u64 len = ftell(file);
    char *contents = allocate(len + 1, sizeof (char));
    contents[len] = 0;
    rewind(file);
    u64 written = fread(contents, sizeof (char), len, file);

    ASSERT(written == len);

    return contents;
}

#include "auxiliary.h"
#include "hashmap.h"

void *check_ptr(void *ptr) {
    if (ptr) return ptr;
    fprintf(stderr, FATAL "Out of memory\n");
    exit(-1);
}

void *heap_alloc(u64 count, u64 size) {
    void *ptr_res = check_ptr(malloc(count * size));

#ifdef EBUG_MEMORY
    printf("* _ -> %016llX\n", ptr_res);
#endif

    return ptr_res;
}

void *heap_realloc(void *ptr, u64 count, u64 size) {
    void *ptr_res = check_ptr(realloc(ptr, count * size));

#ifdef EBUG_MEMORY
    printf("* %016llX -> %016llX\n", ptr, ptr_res);
#endif

    return ptr_res;
}

void heap_dealloc(void *ptr) {
#ifdef EBUG_MEMORY
    printf("* %016llX -> _\n", ptr);
#endif

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
    char *contents = heap_alloc(len + 1, sizeof (char));
    contents[len] = 0;
    rewind(file);
    u64 written = fread(contents, sizeof (char), len, file);

    ASSERT(written == len);

    return contents;
}

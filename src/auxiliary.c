#include "auxiliary.h"

void *check_ptr(void *ptr) {
    if (ptr) return ptr;
    fprintf(stderr, FATAL "Out of memory");
    exit(1);
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

#include <string.h>
#include "bytevec.h"

#define DEFAULT_BYTEVEC_CAP 512

void bytevec_init(ByteVec *bv) {
    bv->cap = DEFAULT_BYTEVEC_CAP;
    bv->len = 0;
    bv->arr = heap_alloc(DEFAULT_BYTEVEC_CAP, sizeof (u8));
}

void bytevec_deinit(ByteVec *bv) {
    heap_dealloc(bv->arr);
}

void bytevec_push_byte(ByteVec *bv, u8 byte) {
    if (bv->len + 1 > bv->cap) {
        bv->cap = bv->cap * 2 + 1;
        bv->arr = heap_realloc(bv->arr, bv->cap, sizeof (u8));
    }

    bv->arr[bv->len++] = byte;
}

void bytevec_push_qword(ByteVec *bv, u64 qword) {
    if (bv->len + 8 > bv->cap) {
        bv->cap = bv->cap * 2 + 8;
        bv->arr = heap_realloc(bv->arr, bv->cap, sizeof (u8));
    }

    memcpy(bv->arr + bv->len, &qword, 8);
    bv->len += 8;
}

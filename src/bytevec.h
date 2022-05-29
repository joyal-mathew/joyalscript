#pragma once

#include "auxiliary.h"

typedef struct {
    u8 *arr;
    u64 len;
    u64 cap;
} ByteVec; // TODO: change the name

void bytevec_init(ByteVec *bv);
void bytevec_deinit(ByteVec *bv);
void bytevec_push_byte(ByteVec *bv, u8 byte);
void bytevec_push_qword(ByteVec *bv, u64 qword);

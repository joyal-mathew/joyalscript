#pragma once

#include "auxiliary.h"
#include "stack.h"

typedef enum {
    at_LabelDef,
    at_LabelRef,
    at_VarDef,
    at_VarRef,
    at_Byte,
    at_Number,
} AtomType;

typedef struct {
    AtomType type;

    union {
        u64 label;
        u64 var_ref;

        struct {
            u64 var;
            u64 val;
        };

        u8 byte;
        u64 number;
    };
} Atom;

typedef struct {
    Stack atoms;
    Stack bytecode;
    u64 uid;
} Assembler;

void assembler_init(Assembler *assembler);
void assembler_deinit(Assembler *assembler);

void assembler_emit(Assembler *assembler, Atom atom);
u64 assembler_get_next(Assembler *assembler);
void assembler_assemble(Assembler *assembler);

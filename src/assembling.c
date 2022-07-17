#include "assembling.h"

void assembler_init(Assembler *assembler) {
    stack_init(&assembler->atoms, sizeof (Atom));
    stack_init(&assembler->bytecode, sizeof (u64));
    assembler->uid = 0;
}

void assembler_deinit(Assembler *assembler) {
    stack_deinit(&assembler->atoms);
    stack_deinit(&assembler->bytecode);
}

void assembler_emit(Assembler *assembler, Atom atom) {
    stack_push(&assembler->atoms, &atom);
}

u64 assembler_get_next(Assembler *assembler) {
    return assembler->uid++;
}

void assembler_assemble(Assembler *assembler) {
    u64 *lookup = heap_alloc(assembler->uid, sizeof (u64));
    u64 pc = 0;

    for (u64 i = 0; i < stack_len(&assembler->atoms); ++i) {
        Atom *atom = stack_index(&assembler->atoms, i);

        switch (atom->type) {
        case at_Byte:
            pc += 1;
            break;
        case at_LabelRef:
        case at_VarRef:
        case at_Number:
            pc += 8;
            break;
        case at_LabelDef:
            lookup[atom->label] = pc;
            break;
        case at_VarDef:
            lookup[atom->var] = atom->val;
            break;
        }
    }

    for (u64 i = 0; i < stack_len(&assembler->atoms); ++i) {
        Atom *atom = stack_index(&assembler->atoms, i);

        switch (atom->type) {
        case at_Byte:
            stack_push_byte(&assembler->bytecode, atom->byte);
            break;
        case at_LabelRef:
            stack_push(&assembler->bytecode, lookup + atom->label);
            break;
        case at_VarRef:
            stack_push(&assembler->bytecode, lookup + atom->var_ref);
            break;
        case at_Number:
            stack_push(&assembler->bytecode, &atom->number);
            break;
        case at_LabelDef:
        case at_VarDef:
            break;
        }
    }

    heap_dealloc(lookup);
}

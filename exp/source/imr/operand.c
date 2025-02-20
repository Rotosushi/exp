/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "imr/operand.h"
#include "utility/unreachable.h"

Operand operand_construct(OperandKind kind, OperandData data) {
    Operand operand_ = {.kind = kind, .data = data};
    return operand_;
}

Operand operand_uninitialized() {
    return operand_construct(OPERAND_UNINITIALIZED, (OperandData){});
}

Operand operand_register(u8 register_) {
    return operand_construct(OPERAND_REGISTER,
                             (OperandData){.register_ = register_});
}
Operand operand_stack(u16 stack) {
    return operand_construct(OPERAND_STACK, (OperandData){.stack = stack});
}

bool operand_equality(Operand A, Operand B) {
    if (A.kind != B.kind) { return false; }

    switch (A.kind) {
    case OPERAND_REGISTER: return A.data.register_ == B.data.register_;
    case OPERAND_STACK:    return A.data.stack == B.data.stack;
    default:               EXP_UNREACHABLE();
    }
}

void print_operand(String *buffer, Operand operand) {
    switch (operand.kind) {
    case OPERAND_REGISTER:
        u8 register_ = operand.data.register_;
        string_append(buffer, SV("r"));
        string_append_u64(buffer, register_);
        break;
    case OPERAND_STACK:
        u16 stack = operand.data.stack;
        string_append(buffer, SV("s"));
        string_append_u64(buffer, stack);
        break;
    default: EXP_UNREACHABLE();
    }
}

/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "imr/operand.h"
#include "env/context.h"
#include "utility/unreachable.h"

Operand operand_construct(OperandKind kind, OperandData data) {
    Operand operand_ = {.kind = kind, .data = data};
    return operand_;
}

bool operand_equality(Operand A, Operand B) {
    if (A.kind != B.kind) { return false; }

    switch (A.kind) {
    case OPERAND_REGISTER: return A.data.register_ == B.data.register_;
    case OPERAND_STACK:    return A.data.stack == B.data.stack;
    default:               EXP_UNREACHABLE();
    }
}

void print_operand(String *buffer, Operand operand, Context *context) {
    switch (operand.kind) {
    case OPERAND_REGISTER:
        u8 register_ = operand.data.register_;
        string_append(buffer, SV("r"));
        string_append_u64(buffer, register_);
        string_append(buffer, SV("["));
        print_scalar(buffer, context_registers_at(context, register_));
        break;
    case OPERAND_CONSTANT:
        Value *constant = context_constants_at(context, operand.data.constant);
        print_value(buffer, constant, context);
        break;
    case OPERAND_SCALAR: print_scalar(buffer, operand.data.scalar); break;
    case OPERAND_LABEL:
        StringView label = context_labels_at(context, operand.data.label);
        string_append(buffer, label);
        break;

    default: EXP_UNREACHABLE();
    }
}

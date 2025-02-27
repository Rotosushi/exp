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

Operand operand_ssa(u32 ssa) {
    Operand operand = {.kind = OPERAND_KIND_SSA, .data.ssa = ssa};
    return operand;
}

Operand operand_scalar(Scalar scalar) {
    Operand operand = {.kind = OPERAND_KIND_SCALAR, .data.scalar = scalar};
    return operand;
}

Operand operand_constant(u32 constant) {
    Operand operand = {.kind          = OPERAND_KIND_CONSTANT,
                       .data.constant = constant};
    return operand;
}

Operand operand_label(u32 label) {
    Operand operand = {.kind = OPERAND_KIND_LABEL, .data.label = label};
    return operand;
}

bool operand_equality(Operand A, Operand B) {
    if (A.kind != B.kind) { return false; }

    switch (A.kind) {
    case OPERAND_KIND_SSA:      return A.data.ssa == B.data.ssa;
    case OPERAND_KIND_CONSTANT: return A.data.constant == B.data.constant;
    case OPERAND_KIND_SCALAR:   return scalar_equal(A.data.scalar, B.data.scalar);
    case OPERAND_KIND_LABEL:    return A.data.label == B.data.label;
    default:                    EXP_UNREACHABLE();
    }
}

void print_operand(String *buffer, Operand operand, Context *context) {
    switch (operand.kind) {
    case OPERAND_KIND_SSA:
        string_append(buffer, SV("%"));
        string_append_u64(buffer, operand.data.ssa);
        break;
    case OPERAND_KIND_CONSTANT:
        Value *constant = context_constants_at(context, operand.data.constant);
        print_value(buffer, constant, context);
        break;
    case OPERAND_KIND_SCALAR: print_scalar(buffer, operand.data.scalar); break;
    case OPERAND_KIND_LABEL:
        StringView label = context_labels_at(context, operand.data.label);
        string_append(buffer, label);
        break;

    default: EXP_UNREACHABLE();
    }
}

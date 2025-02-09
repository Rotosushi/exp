/**
 * Copyright (C) 2024 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stddef.h>

#include "env/context.h"
#include "imr/operand.h"
#include "utility/unreachable.h"

Operand operand_construct(OperandKind kind, OperandData data) {
    Operand operand_ = {.kind = kind, .data = data};
    return operand_;
}

Operand operand_ssa(u32 ssa) {
    Operand operand = {.kind = OPERAND_KIND_SSA, .data.ssa = ssa};
    return operand;
}

Operand operand_i32(i32 i32_) {
    Operand operand = {.kind = OPERAND_KIND_I32, .data.i32_ = i32_};
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
    case OPERAND_KIND_I32:      return A.data.i32_ == B.data.i32_;
    case OPERAND_KIND_LABEL:    return A.data.label == B.data.label;
    default:                    EXP_UNREACHABLE();
    }
}

void print_operand(String *buffer,
                   OperandKind kind,
                   OperandData data,
                   Context *context) {
    switch (kind) {
    case OPERAND_KIND_SSA:
        string_append(buffer, SV("%"));
        string_append_u64(buffer, data.ssa);
        break;
    case OPERAND_KIND_CONSTANT:
        Value *constant = context_constants_at(context, data.constant);
        print_value(buffer, constant, context);
        break;
    case OPERAND_KIND_I32: string_append_i64(buffer, data.i32_); break;
    case OPERAND_KIND_LABEL:
        StringView label = context_labels_at(context, data.label);
        string_append(buffer, label);
        break;

    default: EXP_UNREACHABLE();
    }
}

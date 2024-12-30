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

Operand operand_ssa(u64 ssa) {
    Operand operand = {.kind = OPERAND_KIND_SSA, .data.ssa = ssa};
    return operand;
}

Operand operand_i64(i64 i64_) {
    Operand operand = {.kind = OPERAND_KIND_I64, .data.i64_ = i64_};
    return operand;
}

Operand operand_constant(Value *constant) {
    Operand operand = {.kind          = OPERAND_KIND_CONSTANT,
                       .data.constant = constant};
    return operand;
}

Operand operand_label(ConstantString *label) {
    Operand operand = {.kind = OPERAND_KIND_LABEL, .data.label = label};
    return operand;
}

bool operand_equality(Operand A, Operand B) {
    if (A.kind != B.kind) { return false; }

    switch (A.kind) {
    case OPERAND_KIND_SSA:      return A.data.ssa == B.data.ssa;
    case OPERAND_KIND_CONSTANT: return A.data.constant == B.data.constant;
    case OPERAND_KIND_I64:      return A.data.i64_ == B.data.i64_;
    case OPERAND_KIND_LABEL:    return A.data.label == B.data.label;
    default:                    EXP_UNREACHABLE();
    }
}

static void print_operand_ssa(String *buffer, u64 ssa) {
    string_append(buffer, SV("%"));
    string_append_u64(buffer, ssa);
}

void print_operand(String *buffer,
                   OperandKind kind,
                   OperandData data,
                   Context *context) {
    switch (kind) {
    case OPERAND_KIND_SSA: print_operand_ssa(buffer, data.ssa); break;
    case OPERAND_KIND_CONSTANT:
        print_value(buffer, data.constant, context);
        break;
    case OPERAND_KIND_I64: string_append_i64(buffer, data.i64_); break;
    case OPERAND_KIND_LABEL:
        string_append(buffer, constant_string_to_view(data.label));
        break;

    default: EXP_UNREACHABLE();
    }
}

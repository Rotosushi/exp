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
#include "support/unreachable.h"

Operand operand(OperandKind kind, OperandData data) {
    Operand operand_ = {.kind = kind, .data = data};
    return operand_;
}

Operand operand_ssa(u32 ssa) {
    Operand operand = {.kind = OPERAND_KIND_SSA, .data.ssa = ssa};
    return operand;
}

Operand operand_constant(u32 index) {
    Operand operand = {.kind = OPERAND_KIND_CONSTANT, .data.constant = index};
    return operand;
}

Operand operand_label(u32 index) {
    Operand operand = {.kind = OPERAND_KIND_LABEL, .data.label = index};
    return operand;
}

Operand operand_i64(i64 i64_) {
    Operand operand = {.kind           = OPERAND_KIND_I64,
                       .data.i64_ = i64_};
    return operand;
}

bool operand_equality(Operand A, Operand B) {
    if (A.kind != B.kind) { return false; }

    switch (A.kind) {
    case OPERAND_KIND_SSA:       return A.data.ssa == B.data.ssa;
    case OPERAND_KIND_CONSTANT:  return A.data.constant == B.data.constant;
    case OPERAND_KIND_I64: return A.data.i64_ == B.data.i64_;
    case OPERAND_KIND_LABEL:     return A.data.label == B.data.label;
    default:                     EXP_UNREACHABLE();
    }
}

static void print_operand_ssa(String *restrict string, u32 ssa) {
    string_append(string, SV("%"));
    string_append_u64(string, ssa);
}

static void print_operand_value(String *restrict string,
                                u32 index,
                                Context *restrict context) {
    Value *value = context_constants_at(context, index);
    print_value(string, value, context);
}

static void print_operand_immediate(String *restrict string, i64 immediate) {
    string_append_i64(string, immediate);
}

static void print_operand_label(String *restrict string,
                                u32 index,
                                Context *restrict context) {
    string_append(string, SV("%"));
    StringView name = context_labels_at(context, index);
    string_append(string, name);
}

void print_operand(String *restrict string,
                   Operand operand,
                   Context *restrict context) {
    switch (operand.kind) {
    case OPERAND_KIND_SSA: print_operand_ssa(string, operand.data.ssa); break;
    case OPERAND_KIND_CONSTANT:
        print_operand_value(string, operand.data.constant, context);
        break;
    case OPERAND_KIND_I64:
        print_operand_immediate(string, operand.data.i64_);
        break;
    case OPERAND_KIND_LABEL:
        print_operand_label(string, operand.data.label, context);
        break;

    default: EXP_UNREACHABLE();
    }
}

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

Operand operand_constant(u32 index) {
    Operand operand = {.kind = OPERAND_KIND_CONSTANT, .data.constant = index};
    return operand;
}

Operand operand_i32(i32 immediate) {
    Operand operand = {.kind = OPERAND_KIND_I32, .data.i32_ = immediate};
    return operand;
}

Operand operand_label(u32 index) {
    Operand operand = {.kind = OPERAND_KIND_LABEL, .data.label = index};
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

static void print_operand_ssa(u32 ssa, FILE *restrict file) {
    file_write("%", file);
    file_write_u64(ssa, file);
}

static void
print_operand_value(u32 index, FILE *restrict file, Context *restrict context) {
    Value *value = context_constants_at(context, index);
    print_value(value, file, context);
}

static void print_operand_i32(i32 i32_, FILE *restrict file) {
    file_write_i64(i32_, file);
}

static void
print_operand_label(u32 index, FILE *restrict file, Context *restrict context) {
    file_write("%", file);
    StringView name = context_labels_at(context, index);
    file_write(name.ptr, file);
}

void print_operand(Operand operand,
                   FILE *restrict file,
                   Context *restrict context) {
    switch (operand.kind) {
    case OPERAND_KIND_SSA: print_operand_ssa(operand.data.ssa, file); break;
    case OPERAND_KIND_CONSTANT:
        print_operand_value(operand.data.constant, file, context);
        break;
    case OPERAND_KIND_I32: print_operand_i32(operand.data.i32_, file); break;
    case OPERAND_KIND_LABEL:
        print_operand_label(operand.data.label, file, context);
        break;
        // case OPRFMT_CALL:  print_operand_call(operand.index, file, context);
        // break;

    default: EXP_UNREACHABLE();
    }
}

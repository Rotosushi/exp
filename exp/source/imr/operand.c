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

Operand operand_ssa(u16 ssa) {
    Operand operand = {.kind = OPERAND_KIND_SSA, .ssa = ssa};
    return operand;
}

Operand operand_constant(u16 index) {
    Operand operand = {.kind = OPERAND_KIND_CONSTANT, .index = index};
    return operand;
}

Operand operand_immediate(i16 immediate) {
    Operand operand = {.kind = OPERAND_KIND_IMMEDIATE, .immediate = immediate};
    return operand;
}

Operand operand_label(u16 index) {
    Operand operand = {.kind = OPERAND_KIND_LABEL, .index = index};
    return operand;
}

bool operand_equality(Operand A, Operand B) {
    if (A.kind != B.kind) { return false; }

    switch (A.kind) {
    case OPERAND_KIND_SSA:       return A.ssa == B.ssa;
    case OPERAND_KIND_CONSTANT:  return A.index == B.index;
    case OPERAND_KIND_IMMEDIATE: return A.immediate == B.immediate;
    case OPERAND_KIND_LABEL:     return A.index == B.index;
    default:                     EXP_UNREACHABLE();
    }
}

static void print_operand_ssa(u16 ssa, FILE *restrict file) {
    file_write("%", file);
    file_write_u64(ssa, file);
}

static void
print_operand_value(u16 index, FILE *restrict file, Context *restrict context) {
    Value *value = context_values_at(context, index);
    print_value(value, file, context);
}

static void print_operand_immediate(i16 immediate, FILE *restrict file) {
    file_write_i64(immediate, file);
}

static void
print_operand_label(u16 index, FILE *restrict file, Context *restrict context) {
    file_write("%", file);
    StringView name = context_global_labels_at(context, index);
    file_write(name.ptr, file);
}

void print_operand(Operand operand,
                   FILE *restrict file,
                   Context *restrict context) {
    switch (operand.kind) {
    case OPERAND_KIND_SSA: print_operand_ssa(operand.ssa, file); break;
    case OPERAND_KIND_CONSTANT:
        print_operand_value(operand.index, file, context);
        break;
    case OPERAND_KIND_IMMEDIATE:
        print_operand_immediate(operand.immediate, file);
        break;
    case OPERAND_KIND_LABEL:
        print_operand_label(operand.index, file, context);
        break;
        // case OPRFMT_CALL:  print_operand_call(operand.index, file, context);
        // break;

    default: EXP_UNREACHABLE();
    }
}

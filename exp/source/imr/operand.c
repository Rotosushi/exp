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

Operand operand_ssa(u64 ssa) {
  Operand opr = {.format = OPRFMT_SSA, .ssa = ssa};
  return opr;
}

Operand operand_constant(u64 index) {
  Operand opr = {.format = OPRFMT_VALUE, .index = index};
  return opr;
}

Operand operand_immediate(i64 immediate) {
  Operand opr = {.format = OPRFMT_IMMEDIATE, .immediate = immediate};
  return opr;
}

Operand operand_label(u64 index) {
  Operand opr = {.format = OPRFMT_LABEL, .index = index};
  return opr;
}

/*
Operand operand_call(u64 index) {
  Operand operand = {.format = OPRFMT_CALL, .index = index};
  return operand;
}
*/

bool operand_equality(Operand A, Operand B) {
  if (A.format != B.format) { return false; }
  switch (A.format) {
  case OPRFMT_SSA:       return A.ssa == B.ssa;
  case OPRFMT_VALUE:     return A.index == B.index;
  case OPRFMT_IMMEDIATE: return A.immediate == B.immediate;
  case OPRFMT_LABEL:     return A.index == B.index;

  default: EXP_UNREACHABLE();
  }
}

void print_operand_ssa(u64 ssa,
                       FILE *restrict file,
                       [[maybe_unused]] Context *restrict context) {
  file_write("%", file);
  file_write_u64(ssa, file);
}

static void
print_operand_value(u64 index, FILE *restrict file, Context *restrict context) {
  Value *value = context_values_at(context, index);
  print_value(value, file, context);
}

static void print_operand_immediate(i64 v, FILE *restrict file) {
  file_write_i64(v, file);
}

static void print_operand_global(u64 index,
                                 FILE *restrict file,
                                 Context *restrict context) {
  file_write("@", file);
  StringView name = context_global_labels_at(context, index);
  file_write(name.ptr, file);
}

/*
static void
print_operand_call(u64 index, FILE *restrict file, Context *restrict context) {
  ActualArgumentList *args = context_call_at(context, index);
  file_write("(", file);
  for (u8 i = 0; i < args->size; ++i) {
    Operand arg = args->list[i];
    print_operand(arg, file, context);

    if (i < (args->size - 1)) { file_write(", ", file); }
  }
  file_write(")", file);
}
*/

void print_operand(Operand operand,
                   FILE *restrict file,
                   Context *restrict context) {
  switch (operand.format) {
  case OPRFMT_SSA:   print_operand_ssa(operand.ssa, file, context); break;
  case OPRFMT_VALUE: print_operand_value(operand.index, file, context); break;
  case OPRFMT_IMMEDIATE:
    print_operand_immediate(operand.immediate, file);
    break;
  case OPRFMT_LABEL:
    print_operand_global(operand.index, file, context);
    break;
    // case OPRFMT_CALL:  print_operand_call(operand.index, file, context);
    // break;

  default: EXP_UNREACHABLE();
  }
}

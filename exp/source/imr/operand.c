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
  Operand opr = {.format = OPRFMT_SSA, .value.ssa = ssa};
  return opr;
}

Operand operand_constant(u64 index) {
  Operand opr = {.format = OPRFMT_VALUE, .value.index = index};
  return opr;
}

Operand operand_immediate(i64 immediate) {
  Operand opr = {.format = OPRFMT_IMMEDIATE, .value.immediate = immediate};
  return opr;
}

Operand operand_label(u64 index) {
  Operand opr = {.format = OPRFMT_LABEL, .value.index = index};
  return opr;
}

Operand operand_call(u64 index) {
  Operand operand = {.format = OPRFMT_CALL, .value.index = index};
  return operand;
}

bool operand_equality(Operand A, Operand B) {
  if (A.format != B.format) { return false; }
  switch (A.format) {
  case OPRFMT_SSA:       return A.value.ssa == B.value.ssa;
  case OPRFMT_VALUE:     return A.value.index == B.value.index;
  case OPRFMT_IMMEDIATE: return A.value.immediate == B.value.immediate;
  case OPRFMT_LABEL:     return A.value.index == B.value.index;

  default: unreachable();
  }
}

void print_operand_ssa(u64 ssa,
                       String *restrict out,
                       [[maybe_unused]] Context *restrict context) {
  string_append(out, SV("%"));
  string_append_u64(out, ssa);
}

static void print_operand_value(u64 index,
                                String *restrict out,
                                Context *restrict context) {
  Value *value = context_values_at(context, index);
  print_value(value, out, context);
}

static void print_operand_immediate(
    i64 v, String *restrict out, [[maybe_unused]] Context *restrict context) {
  string_append_i64(out, v);
}

static void print_operand_global(u64 index,
                                 String *restrict out,
                                 Context *restrict context) {
  string_append(out, SV("@"));
  StringView name = context_global_labels_at(context, index);
  string_append(out, name);
}

static void
print_operand_call(u64 index, String *restrict out, Context *restrict context) {
  ActualArgumentList *args = context_call_at(context, index);
  string_append(out, SV("("));
  for (u8 i = 0; i < args->size; ++i) {
    Operand arg = args->list[i];
    print_operand(arg.format, arg.value, out, context);

    if (i < (args->size - 1)) { string_append(out, SV(", ")); }
  }
  string_append(out, SV(")"));
}

void print_operand(OperandFormat format,
                   OperandValue value,
                   String *restrict out,
                   Context *restrict context) {
  switch (format) {
  case OPRFMT_SSA: {
    print_operand_ssa(value.ssa, out, context);
    break;
  }

  case OPRFMT_VALUE: {
    print_operand_value(value.index, out, context);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    print_operand_immediate(value.immediate, out, context);
    break;
  }

  case OPRFMT_LABEL: {
    print_operand_global(value.index, out, context);
    break;
  }

  case OPRFMT_CALL: {
    print_operand_call(value.index, out, context);
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

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

#include "imr/operand.h"
#include "utility/numeric_conversions.h"

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

bool operand_equality(Operand A, Operand B) {
  if (A.format != B.format) { return false; }
  switch (A.format) {
  case OPRFMT_SSA:       return A.ssa == B.ssa;
  case OPRFMT_VALUE:     return A.index == B.index;
  case OPRFMT_IMMEDIATE: return A.immediate == B.immediate;
  case OPRFMT_LABEL:     return A.index == B.index;

  default: unreachable();
  }
}

void print_ssa(u64 v, FILE *restrict file) {
  file_write("SSA[", file);
  print_u64(v, file);
  file_write("]", file);
}

static void print_constant(u64 v, FILE *restrict file) {
  file_write("Constant[", file);
  print_u64(v, file);
  file_write("]", file);
}

static void print_immediate(i64 v, FILE *restrict file) { print_i64(v, file); }

static void print_global(u64 v, FILE *restrict file) {
  file_write("GlobalSymbol[GlobalLabel[", file);
  print_u64(v, file);
  file_write("]]", file);
}

void print_operand(Operand operand, FILE *restrict file) {
  switch (operand.format) {
  case OPRFMT_SSA:       print_ssa(operand.ssa, file); break;
  case OPRFMT_VALUE:     print_constant(operand.index, file); break;
  case OPRFMT_IMMEDIATE: print_immediate(operand.immediate, file); break;
  case OPRFMT_LABEL:     print_global(operand.index, file); break;

  default: file_write("undefined", file);
  }
}

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
#include "imr/operand.h"
#include "utility/numeric_conversions.h"

Operand operand_create(OperandFormat format, u16 common) {
  Operand operand = {.format = format, .common = common};
  return operand;
}

Operand operand_ssa(u16 ssa) {
  Operand opr = {.format = OPRFMT_SSA, .common = ssa};
  return opr;
}

Operand operand_constant(u16 index) {
  Operand opr = {.format = OPRFMT_CONSTANT, .common = index};
  return opr;
}

Operand operand_immediate(u16 imm) {
  Operand opr = {.format = OPRFMT_IMMEDIATE, .common = imm};
  return opr;
}

Operand operand_label(u16 idx) {
  Operand opr = {.format = OPRFMT_LABEL, .common = idx};
  return opr;
}

bool operand_equality(Operand A, Operand B) {
  if (A.format != B.format) { return 0; }
  return A.common == B.common;
}

static void print_local(u16 v, FILE *restrict file) {
  file_write("SSA[", file);
  print_u64(v, file);
  file_write("]", file);
}

static void print_constant(u16 v, FILE *restrict file) {
  file_write("Constant[", file);
  print_u64(v, file);
  file_write("]", file);
}

static void print_immediate(u16 v, FILE *restrict file) {
  print_i64((i16)v, file);
}

static void print_global(u16 v, FILE *restrict file) {
  file_write("GlobalSymbol[", file);
  print_u64(v, file);
  file_write("]", file);
}

void print_operand(Operand operand, FILE *restrict file) {
  switch (operand.format) {
  case OPRFMT_SSA:       print_local(operand.common, file); break;
  case OPRFMT_CONSTANT:  print_constant(operand.common, file); break;
  case OPRFMT_IMMEDIATE: print_immediate(operand.common, file); break;
  case OPRFMT_LABEL:     print_global(operand.common, file); break;

  default: file_write("undefined", file);
  }
}
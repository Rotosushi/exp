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

#include "backend/x64/operand.h"

x64_Operand x64_operand_gpr(u16 gpr) {
  x64_Operand opr = {.format = X64OPRFMT_GPR, .common = gpr};
  return opr;
}

x64_Operand x64_operand_stack(i16 offset) {
  x64_Operand opr = {.format = X64OPRFMT_STACK, .offset = offset};
  return opr;
}

x64_Operand x64_operand_location(x64_Location location) {
  switch (location.kind) {
  case ALLOC_GPR: {
    return x64_operand_gpr(location.gpr);
  }

  case ALLOC_STACK: {
    return x64_operand_stack(location.offset);
  }

  default: unreachable();
  }
}

x64_Operand x64_operand_alloc(x64_Allocation *alloc) {
  return x64_operand_location(alloc->location);
}

x64_Operand x64_operand_constant(u16 idx) {
  x64_Operand opr = {.format = X64OPRFMT_CONSTANT, .common = idx};
  return opr;
}

x64_Operand x64_operand_immediate(u16 value) {
  x64_Operand opr = {.format = X64OPRFMT_IMMEDIATE, .common = value};
  return opr;
}

x64_Operand x64_operand_label(u16 idx) {
  x64_Operand opr = {.format = X64OPRFMT_LABEL, .common = idx};
  return opr;
}
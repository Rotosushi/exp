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

x64_Operand x64_opr_gpr(u16 gpr) {
  x64_Operand opr = {.format = X64OPRFMT_GPR, .common = gpr};
  return opr;
}

x64_Operand x64_opr_stack(u16 offset) {
  x64_Operand opr = {.format = X64OPRFMT_STACK, .common = offset};
  return opr;
}

x64_Operand x64_opr_alloc(x64_Allocation *alloc) {
  switch (alloc->location.kind) {
  case ALLOC_GPR: {
    return x64_opr_gpr(alloc->location.gpr);
  }

  case ALLOC_STACK: {
    return x64_opr_stack(alloc->location.offset);
  }

  default: unreachable();
  }
}

x64_Operand x64_opr_constant(u16 idx) {
  x64_Operand opr = {.format = X64OPRFMT_CONSTANT, .common = idx};
  return opr;
}

x64_Operand x64_opr_immediate(u16 n) {
  x64_Operand opr = {.format = X64OPRFMT_IMMEDIATE, .common = n};
  return opr;
}
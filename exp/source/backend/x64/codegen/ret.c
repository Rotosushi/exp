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
#include "backend/x64/codegen/ret.h"
#include "backend/x64/intrinsics/copy.h"
#include "backend/x64/intrinsics/load.h"
#include "utility/unreachable.h"

void x64_codegen_ret(Instruction I, u64 Idx, x64_Context *restrict context) {
  x64_FunctionBody *body = current_x64_body(context);
  switch (I.B_format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_context_allocation_of(context, I.B.ssa);
    if (x64_allocation_location_eq(B, body->result->location)) { break; }
    x64_codegen_copy_allocation(body->result, B, Idx, context);
    break;
  }

  case OPRFMT_VALUE: {
    x64_codegen_load_allocation_from_value(
        body->result, I.B.index, Idx, context);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_context_append(context,
                       x64_mov(x64_operand_alloc(body->result),
                               x64_operand_immediate(I.B.immediate)));
    break;
  }

  case OPRFMT_LABEL: {
    x64_Address label = x64_address_from_label(I.B.index);
    x64_codegen_copy_allocation_from_memory(
        body->result, &label, body->result->type, Idx, context);
    break;
  }

  default: EXP_UNREACHABLE;
  }

  x64_context_append(
      context,
      x64_mov(x64_operand_gpr(X64GPR_RSP), x64_operand_gpr(X64GPR_RBP)));
  x64_context_append(context, x64_pop(x64_operand_gpr(X64GPR_RBP)));
  x64_context_append(context, x64_ret());
}

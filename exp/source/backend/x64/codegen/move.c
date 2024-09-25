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
#include "backend/x64/codegen/move.h"
#include "backend/x64/intrinsics/load.h"
#include "utility/unreachable.h"

static void
x64_codegen_move_to_ssa(Instruction I, u64 Idx, x64_Context *restrict context) {
  LocalVariable *local = x64_context_lookup_ssa(context, I.A.ssa);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_context_allocation_of(context, I.B.ssa);
    x64_context_allocate_from_active(context, local, B, Idx);
    break;
  }

  case OPRFMT_VALUE: {
    x64_Allocation *A = x64_context_allocate(context, local, Idx);
    x64_codegen_load_allocation_from_value(A, I.B.index, Idx, context);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_Allocation *A = x64_context_allocate(context, local, Idx);
    x64_context_append(
        context,
        x64_mov(x64_operand_alloc(A), x64_operand_immediate(I.B.immediate)));
    break;
  }

  case OPRFMT_LABEL:
  default:           EXP_UNREACHABLE;
  }
}

void x64_codegen_move(Instruction I, u64 Idx, x64_Context *restrict context) {
  switch (I.A.format) {
  case OPRFMT_SSA: {
    x64_codegen_move_to_ssa(I, Idx, context);
    break;
  }

  case OPRFMT_LABEL: {
    x64_codegen_move_to_label(I, Idx, context);
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

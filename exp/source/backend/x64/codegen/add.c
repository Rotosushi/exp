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

#include "backend/x64/codegen/add.h"
#include "utility/unreachable.h"

void x64_codegen_add(Instruction I, u64 Idx, x64_Context *restrict context) {
  LocalVariable *local = x64_context_lookup_ssa(context, I.A);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_context_allocation_of(context, I.B.ssa);
    switch (I.C.format) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_context_allocation_of(context, I.C.ssa);
      // if B or C is in a gpr we use it as the allocation point of A
      // and the destination operand of the x64 add instruction.
      // this is to try and keep the result, A, in a register.
      if (B->location.kind == LOCATION_GPR) {
        x64_Allocation *A =
            x64_context_allocate_from_active(context, local, B, Idx);
        x64_context_append(context,
                           x64_add(x64_operand_alloc(A), x64_operand_alloc(C)));
        return;
      }

      if (C->location.kind == LOCATION_GPR) {
        x64_Allocation *A =
            x64_context_allocate_from_active(context, local, C, Idx);
        x64_context_append(context,
                           x64_add(x64_operand_alloc(A), x64_operand_alloc(B)));
        return;
      }

      // since B and C are memory operands we have to move B or C
      // to a reg and then add.
      x64_GPR gpr       = x64_context_aquire_any_gpr(context, Idx);
      x64_Allocation *A = x64_context_allocate_to_gpr(context, local, gpr, Idx);

      // we use the huristic of longest lifetime to choose
      // which of B and C to move into A's gpr.
      if (B->lifetime.last_use <= C->lifetime.last_use) {
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A), x64_operand_alloc(C)));
        x64_context_append(context,
                           x64_add(x64_operand_alloc(A), x64_operand_alloc(B)));
      } else {
        x64_context_append(context,
                           x64_mov(x64_operand_alloc(A), x64_operand_alloc(B)));
        x64_context_append(context,
                           x64_add(x64_operand_alloc(A), x64_operand_alloc(C)));
      }
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_Allocation *A =
          x64_context_allocate_from_active(context, local, B, Idx);

      x64_context_append(
          context,
          x64_add(x64_operand_alloc(A), x64_operand_immediate(I.C.immediate)));
      break;
    }

    case OPRFMT_LABEL:
    case OPRFMT_VALUE:
    default:           EXP_UNREACHABLE;
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    switch (I.C.format) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_context_allocation_of(context, I.C.ssa);
      x64_Allocation *A =
          x64_context_allocate_from_active(context, local, C, Idx);

      x64_context_append(
          context,
          x64_add(x64_operand_alloc(A), x64_operand_immediate(I.B.immediate)));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_Allocation *A = x64_context_allocate(context, local, Idx);
      x64_context_append(
          context,
          x64_mov(x64_operand_alloc(A), x64_operand_immediate(I.B.immediate)));
      x64_context_append(
          context,
          x64_add(x64_operand_alloc(A), x64_operand_immediate(I.C.immediate)));
      break;
    }

    case OPRFMT_LABEL:
    case OPRFMT_VALUE:
    default:           EXP_UNREACHABLE;
    }
    break;
  }

  case OPRFMT_LABEL:
  case OPRFMT_VALUE:
  default:           EXP_UNREACHABLE;
  }
}
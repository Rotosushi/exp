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

#include "backend/x64/codegen/mod.h"
#include "utility/unreachable.h"

void x64_codegen_mod(Instruction I, u64 Idx, x64_Context *restrict context) {
  LocalVariable *local = x64_context_lookup_ssa(context, I.A);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_context_allocation_of(context, I.B.ssa);
    switch (I.C.format) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_context_allocation_of(context, I.C.ssa);
      if ((B->location.kind == LOCATION_GPR) &&
          (B->location.gpr == X64GPR_RAX)) {
        x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

        x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
        break;
      }

      if ((C->location.kind == LOCATION_GPR) &&
          (C->location.gpr == X64GPR_RAX)) {
        x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);

        x64_context_reallocate_active(context, C);

        x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

        x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
        break;
      }

      x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
      x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
      x64_context_append(
          context, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

      x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
      x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
      x64_context_append(
          context, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

      x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);
      x64_context_append(
          context,
          x64_mov(x64_operand_gpr(gpr), x64_operand_immediate(I.C.immediate)));

      x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));
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
      x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
      x64_context_append(
          context,
          x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

      x64_Allocation *C = x64_context_allocation_of(context, I.C.ssa);
      if ((C->location.kind == LOCATION_GPR) &&
          (C->location.gpr == X64GPR_RAX)) {
        x64_context_reallocate_active(context, C);
      }

      x64_context_append(context,
                         x64_mov(x64_operand_gpr(X64GPR_RAX),
                                 x64_operand_immediate(I.B.immediate)));
      x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
      x64_context_append(
          context,
          x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

      x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
      x64_context_append(context,
                         x64_mov(x64_operand_gpr(X64GPR_RAX),
                                 x64_operand_immediate(I.B.immediate)));

      x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);
      x64_context_append(
          context,
          x64_mov(x64_operand_gpr(gpr), x64_operand_immediate(I.C.immediate)));

      x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));
      x64_context_release_gpr(context, gpr, Idx);
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

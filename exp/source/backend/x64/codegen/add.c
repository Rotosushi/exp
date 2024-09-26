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
#include "backend/x64/intrinsics/copy.h"
#include "utility/unreachable.h"

static void x64_codegen_add_ssa(Instruction I,
                                LocalVariable *local,
                                u64 Idx,
                                x64_Context *restrict context) {
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

  case OPRFMT_LABEL: {
    x64_Address label = x64_address_from_label(I.C.index);
    x64_Allocation *A =
        x64_context_allocate_from_active(context, local, B, Idx);

    if (A->location.kind == LOCATION_GPR) {
      x64_context_append(
          context, x64_add(x64_operand_alloc(A), x64_operand_address(label)));
    } else {
      x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);

      x64_context_append(
          context, x64_mov(x64_operand_gpr(gpr), x64_operand_address(label)));
      x64_context_append(context,
                         x64_add(x64_operand_alloc(A), x64_operand_gpr(gpr)));

      x64_context_release_gpr(context, gpr, Idx);
    }
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

static void x64_codegen_add_immediate(Instruction I,
                                      LocalVariable *restrict local,
                                      u64 Idx,
                                      x64_Context *restrict context) {
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

  case OPRFMT_LABEL: {
    x64_Allocation *A = x64_context_allocate(context, local, Idx);
    x64_Address label = x64_address_from_label(I.C.index);
    x64_codegen_copy_allocation_from_memory(A, &label, A->type, Idx, context);
    x64_context_append(
        context,
        x64_add(x64_operand_alloc(A), x64_operand_immediate(I.C.immediate)));
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

static void x64_codegen_add_label(Instruction I,
                                  LocalVariable *restrict local,
                                  u64 Idx,
                                  x64_Context *restrict context) {
  x64_Address label = x64_address_from_label(I.B.index);
  switch (I.C.format) {
  case OPRFMT_SSA: {
    x64_Allocation *C = x64_context_allocation_of(context, I.C.ssa);
    x64_Allocation *A =
        x64_context_allocate_from_active(context, local, C, Idx);
    if (A->location.kind == LOCATION_GPR) {
      x64_context_append(
          context, x64_add(x64_operand_alloc(A), x64_operand_address(label)));
    } else {
      x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);
      x64_context_append(
          context, x64_mov(x64_operand_gpr(gpr), x64_operand_address(label)));
      x64_context_append(context,
                         x64_add(x64_operand_alloc(A), x64_operand_gpr(gpr)));
      x64_context_release_gpr(context, gpr, Idx);
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_Allocation *A = x64_context_allocate(context, local, Idx);
    x64_context_append(
        context,
        x64_mov(x64_operand_alloc(A), x64_operand_immediate(I.C.immediate)));
    x64_context_append(
        context, x64_add(x64_operand_alloc(A), x64_operand_address(label)));
    break;
  }

  case OPRFMT_LABEL: {
    x64_Allocation *A   = x64_context_allocate(context, local, Idx);
    x64_Address label_C = x64_address_from_label(I.C.index);
    x64_codegen_copy_allocation_from_memory(A, &label_C, A->type, Idx, context);

    if (A->location.kind == LOCATION_GPR) {
      x64_context_append(
          context, x64_add(x64_operand_alloc(A), x64_operand_address(label)));
    } else {
      x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);

      x64_context_append(
          context, x64_mov(x64_operand_gpr(gpr), x64_operand_address(label)));
      x64_context_append(context,
                         x64_add(x64_operand_alloc(A), x64_operand_gpr(gpr)));

      x64_context_release_gpr(context, gpr, Idx);
    }
  }

  default: EXP_UNREACHABLE;
  }
}

void x64_codegen_add(Instruction I, u64 Idx, x64_Context *restrict context) {
  LocalVariable *local = x64_context_lookup_ssa(context, I.A.ssa);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_codegen_add_ssa(I, local, Idx, context);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_codegen_add_immediate(I, local, Idx, context);
    break;
  }

  case OPRFMT_LABEL: {
    x64_codegen_add_label(I, local, Idx, context);
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

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
#include "backend/x64/intrinsics/address_of.h"
#include "utility/unreachable.h"

static void x64_codegen_mod_ssa(Instruction I,
                                LocalVariable *restrict local,
                                u64 Idx,
                                x64_Context *restrict context) {
  x64_Allocation *B = x64_context_allocation_of(context, I.B.ssa);
  switch (I.C_format) {
  case OPRFMT_SSA: {
    x64_Allocation *C = x64_context_allocation_of(context, I.C.ssa);
    if (x64_location_eq(B->location, x64_location_gpr(X64GPR_RAX))) {
      x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
      x64_context_append(
          context,
          x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

      x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
      break;
    }

    if (x64_location_eq(C->location, x64_location_gpr(X64GPR_RAX))) {
      x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);

      x64_context_reallocate_active(context, C);

      x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
      x64_context_append(
          context, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

      x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
      x64_context_release_gpr(context, X64GPR_RAX, Idx);
      break;
    }

    x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
    x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
    x64_context_append(
        context, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

    x64_context_append(context, x64_idiv(x64_operand_alloc(C)));
    x64_context_release_gpr(context, X64GPR_RAX, Idx);

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
    x64_context_release_gpr(context, X64GPR_RAX, Idx);
    break;
  }

  case OPRFMT_LABEL: {
    x64_Address C = x64_address_of_global(I.C.index, Idx, context);
    x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
    x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
    x64_context_append(
        context, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

    x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);
    x64_context_append(context,
                       x64_mov(x64_operand_gpr(gpr), x64_operand_address(C)));

    x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));
    x64_context_release_gpr(context, X64GPR_RAX, Idx);
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

static void x64_codegen_mod_immediate(Instruction I,
                                      LocalVariable *restrict local,
                                      u64 Idx,
                                      x64_Context *restrict context) {
  switch (I.C_format) {
  case OPRFMT_SSA: {
    x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
    x64_context_append(
        context,
        x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

    x64_Allocation *C = x64_context_allocation_of(context, I.C.ssa);
    if (x64_location_eq(C->location, x64_location_gpr(X64GPR_RAX))) {
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

  case OPRFMT_LABEL: {
    x64_Address C = x64_address_of_global(I.C.index, Idx, context);
    x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
    x64_context_append(
        context,
        x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

    x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
    x64_context_append(context,
                       x64_mov(x64_operand_gpr(X64GPR_RAX),
                               x64_operand_immediate(I.B.immediate)));

    x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);
    x64_context_append(context,
                       x64_mov(x64_operand_gpr(gpr), x64_operand_address(C)));

    x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));
    x64_context_release_gpr(context, gpr, Idx);
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

static void x64_codegen_mod_label(Instruction I,
                                  LocalVariable *restrict local,
                                  u64 Idx,
                                  x64_Context *restrict context) {
  x64_Address B = x64_address_of_global(I.B.index, Idx, context);
  switch (I.C_format) {
  case OPRFMT_SSA: {
    x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
    x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
    x64_context_append(
        context, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_address(B)));

    x64_Allocation *C = x64_context_allocation_of(context, I.C.ssa);
    if (x64_location_eq(C->location, x64_location_gpr(X64GPR_RAX))) {
      x64_context_reallocate_active(context, C);
    }

    x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);
    x64_context_append(context,
                       x64_mov(x64_operand_gpr(gpr), x64_operand_alloc(C)));

    x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));
    x64_context_release_gpr(context, X64GPR_RAX, Idx);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
    x64_context_append(
        context,
        x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

    x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
    x64_context_append(
        context, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_address(B)));

    x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);
    x64_context_append(
        context,
        x64_mov(x64_operand_gpr(gpr), x64_operand_immediate(I.C.immediate)));

    x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));
    x64_context_release_gpr(context, gpr, Idx);
    break;
  }

  case OPRFMT_LABEL: {
    x64_Address C = x64_address_of_global(I.C.index, Idx, context);
    x64_context_allocate_to_gpr(context, local, X64GPR_RDX, Idx);
    x64_context_aquire_gpr(context, X64GPR_RAX, Idx);
    x64_context_append(
        context, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_address(B)));

    x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);
    x64_context_append(context,
                       x64_mov(x64_operand_gpr(gpr), x64_operand_address(C)));

    x64_context_append(context, x64_idiv(x64_operand_gpr(gpr)));
    x64_context_release_gpr(context, X64GPR_RAX, Idx);

    break;
  }

  default: EXP_UNREACHABLE;
  }
}

void x64_codegen_mod(Instruction I, u64 Idx, x64_Context *restrict context) {
  LocalVariable *local = x64_context_lookup_ssa(context, I.A.ssa);
  switch (I.B_format) {
  case OPRFMT_SSA: {
    x64_codegen_mod_ssa(I, local, Idx, context);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_codegen_mod_immediate(I, local, Idx, context);
    break;
  }

  case OPRFMT_LABEL: {
    x64_codegen_mod_label(I, local, Idx, context);
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

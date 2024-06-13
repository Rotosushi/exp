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
#include <assert.h>
#include <stdlib.h>

#include "backend/x64/allocator.h"
#include "backend/x64/codegen.h"
#include "backend/x64/context.h"
#include "backend/x64/emit.h"

static void x64gen_ret(Instruction I,
                       [[maybe_unused]] u16 Idx,
                       x64_Bytecode *restrict x64bc,
                       [[maybe_unused]] LocalVariables *restrict locals,
                       x64_Allocator *restrict allocator) {
  // since we are returning, we know by definition
  // that lifetimes are ending. thus we can just emit
  // mov's where necessary.
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    // #TODO this needs to check that the result is in the return allocation
    // not the hardcoded %rax location
    if ((B->location.kind == ALLOC_GPR) && (B->location.gpr == X64GPR_RAX)) {
      break;
    }

    x64_bytecode_append_mov(x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));
    break;
  }

  case OPRFMT_CONSTANT: {
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_constant(I.B));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_immediate(I.B));
    break;
  }

  default: unreachable();
  }

  x64_bytecode_append_mov(
      x64bc, x64_opr_gpr(X64GPR_RSP), x64_opr_gpr(X64GPR_RBP));
  x64_bytecode_append_pop(x64bc, x64_opr_gpr(X64GPR_RBP));
  x64_bytecode_append_ret(x64bc);
}

static void x64gen_load(Instruction I,
                        u16 Idx,
                        x64_Bytecode *restrict x64bc,
                        LocalVariables *restrict locals,
                        x64_Allocator *restrict allocator) {
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  x64_Allocation *A    = x64_allocator_allocate(allocator, Idx, local, x64bc);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    if ((A->location.kind == ALLOC_STACK) &&
        (B->location.kind == ALLOC_STACK)) {
      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);

      x64_bytecode_append_mov(x64bc, x64_opr_gpr(gpr), x64_opr_alloc(B));
      x64_bytecode_append_mov(x64bc, x64_opr_alloc(A), x64_opr_gpr(gpr));
      break;
    }

    x64_bytecode_append_mov(x64bc, x64_opr_alloc(A), x64_opr_alloc(B));
    break;
  }

  case OPRFMT_CONSTANT: {
    x64_bytecode_append_mov(x64bc, x64_opr_alloc(A), x64_opr_constant(I.B));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_bytecode_append_mov(x64bc, x64_opr_alloc(A), x64_opr_immediate(I.B));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_neg(Instruction I,
                       u16 Idx,
                       x64_Bytecode *restrict x64bc,
                       LocalVariables *restrict locals,
                       x64_Allocator *restrict allocator) {
  // assert that we don't generate trivially foldable instructions
  assert(I.Bfmt == OPRFMT_SSA);
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  x64_Allocation *B    = x64_allocator_allocation_of(allocator, I.B);
  x64_Allocation *A =
      x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

  x64_bytecode_append_neg(x64bc, x64_opr_alloc(A));
}

static void x64gen_add(Instruction I,
                       u16 Idx,
                       x64_Bytecode *restrict x64bc,
                       LocalVariables *restrict locals,
                       x64_Allocator *restrict allocator) {
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
      // if B or C is in a gpr we use it as the allocation point of A
      // and the destination operand of the x64 add instruction.
      // this is to try and keep the result, A, in a register.
      if (B->location.kind == ALLOC_GPR) {
        x64_Allocation *A =
            x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);
        x64_bytecode_append_add(x64bc, x64_opr_alloc(A), x64_opr_alloc(C));
        return;
      }

      if (C->location.kind == ALLOC_GPR) {
        x64_Allocation *A =
            x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);
        x64_bytecode_append_add(x64bc, x64_opr_alloc(A), x64_opr_alloc(B));
        return;
      }

      // since B and C are memory operands we have to move B or C
      // to a reg and then add.
      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_Allocation *A =
          x64_allocator_allocate_to_gpr(allocator, gpr, Idx, local, x64bc);

      // we use the huristic of longest lifetime to choose
      // which of B and C to move into A's gpr.
      if (B->lifetime.last_use <= C->lifetime.last_use) {
        x64_bytecode_append_mov(x64bc, x64_opr_alloc(A), x64_opr_alloc(C));
        x64_bytecode_append_add(x64bc, x64_opr_alloc(A), x64_opr_alloc(B));
      } else {
        x64_bytecode_append_mov(x64bc, x64_opr_alloc(A), x64_opr_alloc(B));
        x64_bytecode_append_add(x64bc, x64_opr_alloc(A), x64_opr_alloc(C));
      }
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append_add(x64bc, x64_opr_alloc(A), x64_opr_constant(I.C));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append_add(x64bc, x64_opr_alloc(A), x64_opr_immediate(I.C));
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    x64_Allocation *A =
        x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

    x64_bytecode_append_add(x64bc, x64_opr_alloc(A), x64_opr_constant(I.B));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    x64_Allocation *A =
        x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

    x64_bytecode_append_add(x64bc, x64_opr_alloc(A), x64_opr_immediate(I.B));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_sub(Instruction I,
                       u16 Idx,
                       x64_Bytecode *restrict x64bc,
                       LocalVariables *restrict locals,
                       x64_Allocator *restrict allocator) {
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
      // since subtraction is not commutative we have to allocate A from B
      // regardless of which of B or C is in a register.
      if ((B->location.kind == ALLOC_GPR) || (C->location.kind == ALLOC_GPR)) {
        x64_Allocation *A =
            x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_bytecode_append_sub(x64bc, x64_opr_alloc(A), x64_opr_alloc(C));
        return;
      }

      /*
        #NOTE there is no sub instruction which takes two memory operands.
        so we have to emit a mov to get either B or C into a register
        then emit a sub instruction.
        we choose to move B into a register, simply because it makes it
        easy to reuse exising allocator functions.

        we can alternatively move C into a register and allocate A from B.
        but when would we want to do this? This doesn't avoid a mov.
        (that could only be accomplished be allocating B or C differently)
        it leaves the result in memory, which makes it much more likely
        that a future usage of A will result in emitting a mov instruction.
        I don't know, something to ponder.
      */
      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_Allocation *A =
          x64_allocator_allocate_to_gpr(allocator, gpr, Idx, local, x64bc);

      x64_bytecode_append_mov(x64bc, x64_opr_alloc(A), x64_opr_alloc(B));

      x64_bytecode_append_sub(x64bc, x64_opr_alloc(A), x64_opr_alloc(C));
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append_sub(x64bc, x64_opr_alloc(A), x64_opr_constant(I.C));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append_sub(x64bc, x64_opr_alloc(A), x64_opr_immediate(I.C));
      break;
    }

    default: unreachable();
    }
    break;
  }

  /*
    #NOTE: there is no x64 sub instruction which takes an
    immediate value on the lhs. so we have to move the
    value of B into a gpr and allocate A there.
    Then we can emit the sub instruction.

    #NOTE that we could also aquire a stack slot for the immediate
    value, and emit the sub. this is a single instruction if and
    only if C was in a gpr; otherwise we are forced to emit a mov
    to ensure one operand of the sub is in a gpr.
    and, as above, this leaves the result in memory,
    which makes it more likely that a future usage will result
    in generating another mov instruction.

    I don't know the difference in timings for the various forms
    of the sub instruction, my assumption is (reg,reg) is faster than
    either (reg/mem) or (mem/reg); and relatively equal to (reg,imm).

  */
  case OPRFMT_CONSTANT: {
    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);

    x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
    x64_bytecode_append_mov(x64bc, x64_opr_gpr(gpr), x64_opr_constant(I.B));
    x64_Allocation *A =
        x64_allocator_allocate_to_gpr(allocator, gpr, Idx, local, x64bc);

    x64_bytecode_append_sub(x64bc, x64_opr_alloc(A), x64_opr_alloc(C));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);

    x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
    x64_bytecode_append_mov(x64bc, x64_opr_gpr(gpr), x64_opr_immediate(I.B));
    x64_Allocation *A =
        x64_allocator_allocate_to_gpr(allocator, gpr, Idx, local, x64bc);

    x64_bytecode_append_sub(x64bc, x64_opr_alloc(A), x64_opr_alloc(C));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_mul(Instruction I,
                       u16 Idx,
                       x64_Bytecode *restrict x64bc,
                       LocalVariables *restrict locals,
                       x64_Allocator *restrict allocator) {
  /*
    imul takes a single reg/mem argument,
    and expects the other argument to be in %rax
    and stores the result in %rdx:%rax.
  */
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
      if ((B->location.kind == ALLOC_GPR) && (B->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);

        x64_bytecode_append_imul(x64bc, x64_opr_alloc(C));
        break;
      }

      if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);

        x64_bytecode_append_imul(x64bc, x64_opr_alloc(B));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      if ((B->lifetime.last_use <= C->lifetime.last_use)) {
        x64_bytecode_append_mov(
            x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));
        x64_bytecode_append_imul(x64bc, x64_opr_alloc(C));
      } else {
        x64_bytecode_append_mov(
            x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(C));
        x64_bytecode_append_imul(x64bc, x64_opr_alloc(B));
      }
      break;
    }

    case OPRFMT_CONSTANT: {
      if (x64_allocation_location_eq(B, x64_location_reg(X64GPR_RAX))) {
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_constant(I.C));
        x64_bytecode_append_imul(x64bc, x64_opr_alloc(B));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_constant(I.C));
      x64_bytecode_append_imul(x64bc, x64_opr_alloc(B));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      if (x64_allocation_location_eq(B, x64_location_reg(X64GPR_RAX))) {
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(I.C));
        x64_bytecode_append_imul(x64bc, x64_opr_alloc(B));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_immediate(I.C));
      x64_bytecode_append_imul(x64bc, x64_opr_alloc(B));
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if (x64_allocation_location_eq(C, x64_location_reg(X64GPR_RAX))) {
      x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_constant(I.B));
      x64_bytecode_append_imul(x64bc, x64_opr_alloc(C));
      break;
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_constant(I.B));
    x64_bytecode_append_imul(x64bc, x64_opr_alloc(C));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(I.B));
      x64_bytecode_append_imul(x64bc, x64_opr_alloc(C));
      break;
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_immediate(I.B));
    x64_bytecode_append_imul(x64bc, x64_opr_alloc(C));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_div(Instruction I,
                       u16 Idx,
                       x64_Bytecode *restrict x64bc,
                       LocalVariables *restrict locals,
                       x64_Allocator *restrict allocator) {
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
      if ((B->location.kind == ALLOC_GPR) && (B->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

        x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));
        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        break;
      }

      if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

        x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

        x64_allocator_reallocate_active(allocator, C, x64bc);

        x64_bytecode_append_mov(
            x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));

        x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));
        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

      x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

      x64_bytecode_append_mov(x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));

      x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));
      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append_mov(x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));

      x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append_mov(x64bc, x64_opr_gpr(gpr), x64_opr_constant(I.C));

      x64_bytecode_append_idiv(x64bc, x64_opr_gpr(gpr));

      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append_mov(x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));

      x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append_mov(x64bc, x64_opr_gpr(gpr), x64_opr_immediate(I.C));

      x64_bytecode_append_idiv(x64bc, x64_opr_gpr(gpr));

      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_constant(I.B));
    x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));

    x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_immediate(I.B));
    x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));

    x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    break;
  }

  default: unreachable();
  }
}

static void x64gen_mod(Instruction I,
                       u16 Idx,
                       x64_Bytecode *restrict x64bc,
                       LocalVariables *restrict locals,
                       x64_Allocator *restrict allocator) {
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
      if ((B->location.kind == ALLOC_GPR) && (B->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

        x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));
        break;
      }

      if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);

        x64_allocator_reallocate_active(allocator, C, x64bc);

        x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));

        x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
      x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64_bytecode_append_mov(x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));

      x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
      x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64_bytecode_append_mov(x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append_mov(x64bc, x64_opr_gpr(gpr), x64_opr_constant(I.C));

      x64_bytecode_append_idiv(x64bc, x64_opr_gpr(gpr));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
      x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64_bytecode_append_mov(x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_alloc(B));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append_mov(x64bc, x64_opr_gpr(gpr), x64_opr_immediate(I.C));

      x64_bytecode_append_idiv(x64bc, x64_opr_gpr(gpr));
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_constant(I.B));
    x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RDX), x64_opr_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_bytecode_append_mov(
        x64bc, x64_opr_gpr(X64GPR_RAX), x64_opr_immediate(I.B));
    x64_bytecode_append_idiv(x64bc, x64_opr_alloc(C));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_bytecode(Bytecode *restrict bc,
                            x64_Bytecode *restrict x64bc,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  for (u16 idx = 0; idx < bc->length; ++idx) {
    Instruction I = bc->buffer[idx];

    switch (I.opcode) {
    case OPC_RET: {
      x64gen_ret(I, idx, x64bc, locals, allocator);
      break;
    }

    case OPC_LOAD: {
      x64gen_load(I, idx, x64bc, locals, allocator);
      break;
    }

    case OPC_NEG: {
      x64gen_neg(I, idx, x64bc, locals, allocator);
      break;
    }

    case OPC_ADD: {
      x64gen_add(I, idx, x64bc, locals, allocator);
      break;
    }

    case OPC_SUB: {
      x64gen_sub(I, idx, x64bc, locals, allocator);
      break;
    }

    case OPC_MUL: {
      x64gen_mul(I, idx, x64bc, locals, allocator);
      break;
    }

    case OPC_DIV: {
      x64gen_div(I, idx, x64bc, locals, allocator);
      break;
    }

    case OPC_MOD: {
      x64gen_mod(I, idx, x64bc, locals, allocator);
      break;
    }

    default: unreachable();
    }
  }
}

static void x64gen_function_header(x64_Allocator *restrict allocator,
                                   x64_Bytecode *restrict x64bc) {
  if (x64_allocator_uses_stack(allocator)) {
    x64_bytecode_prepend_sub(
        x64bc,
        x64_opr_gpr(X64GPR_RSP),
        x64_opr_immediate(x64_allocator_total_stack_size(allocator)));
  }
  x64_bytecode_prepend_mov(
      x64bc, x64_opr_gpr(X64GPR_RBP), x64_opr_gpr(X64GPR_RSP));
  x64_bytecode_prepend_push(x64bc, x64_opr_gpr(X64GPR_RBP));
}

static void x64gen_function(FunctionBody *restrict body,
                            x64_FunctionBody *restrict x64body) {
  LocalVariables *locals  = &body->locals;
  Bytecode *bc            = &body->bc;
  x64_Bytecode *x64bc     = &x64body->bc;
  x64_Allocator allocator = x64_allocator_create(body);

  x64gen_bytecode(bc, x64bc, locals, &allocator);

  x64body->stack_size = x64_allocator_total_stack_size(&allocator);
  x64gen_function_header(&allocator, x64bc);

  x64_allocator_destroy(&allocator);
}

static void x64gen_ste(SymbolTableElement *restrict ste,
                       x64_Context *restrict x64context) {
  StringView name       = ste->name;
  x64_Symbol *x64symbol = x64_context_symbol(x64context, name);

  switch (ste->kind) {
  case STE_UNDEFINED: {
    // #TODO this should lower to a forward declaration
    break;
  }

  case STE_FUNCTION: {
    FunctionBody *body        = &ste->function_body;
    x64_FunctionBody *x64body = &x64symbol->body;
    *x64body                  = x64_function_body_create(body->arguments.size);
    x64gen_function(body, x64body);
    break;
  }

  default: unreachable();
  }
}

void x64_codegen(Context *restrict context) {
  x64_Context x64context = x64_context_create(context);

  SymbolTableIterator iter = context_global_symbol_table_iterator(context);

  while (!symbol_table_iterator_done(&iter)) {
    x64gen_ste(iter.element, &x64context);

    symbol_table_iterator_next(&iter);
  }

  x64_emit(&x64context);
  x64_context_destroy(&x64context);
}
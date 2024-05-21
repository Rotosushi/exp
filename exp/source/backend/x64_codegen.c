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

#include "backend/x64_allocator.h"
#include "backend/x64_codegen.h"
#include "backend/x64_context.h"
#include "backend/x64_emit.h"

static void x64gen_ret(Instruction I,
                       X64Bytecode *restrict x64bc,
                       X64Allocator *restrict allocator) {
  // since we are returning, we know by definition
  // that lifetimes are ending. thus we can just emit
  // mov's if necessary.
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    X64ActiveAllocation B = *x64allocator_allocation_of(allocator, I.B);
    // #TODO this needs to check that the result is in the return allocation
    // not the hardcoded %rax location
    if ((B.allocation.kind == ALLOC_GPR) && (B.allocation.gpr == X64GPR_RAX)) {
      break;
    }

    x64bytecode_append_mov(
        x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));
    break;
  }

  case OPRFMT_CONSTANT: {
    x64bytecode_append_mov(x64bc, x64opr_gpr(X64GPR_RAX), x64opr_constant(I.B));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64bytecode_append_mov(
        x64bc, x64opr_gpr(X64GPR_RAX), x64opr_immediate(I.B));
    break;
  }

  default: unreachable();
  }

  x64bytecode_append_mov(x64bc, x64opr_gpr(X64GPR_RSP), x64opr_gpr(X64GPR_RBP));
  x64bytecode_append_pop(x64bc, x64opr_gpr(X64GPR_RBP));
  x64bytecode_append_ret(x64bc);
}

static void x64gen_move(Instruction I,
                        u16 Idx,
                        X64Bytecode *restrict x64bc,
                        X64Allocator *restrict allocator) {
  X64ActiveAllocation A = x64allocator_allocate(allocator, Idx, I.A, x64bc);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    X64ActiveAllocation B = *x64allocator_allocation_of(allocator, I.B);
    if ((A.allocation.kind == ALLOC_STACK) &&
        (B.allocation.kind == ALLOC_STACK)) {
      X64GPR gpr = x64allocator_aquire_any_gpr(allocator, Idx, x64bc);

      x64bytecode_append_mov(
          x64bc, x64opr_gpr(gpr), x64opr_alloc(&B.allocation));
      x64bytecode_append_mov(
          x64bc, x64opr_alloc(&A.allocation), x64opr_gpr(gpr));
      break;
    }

    x64bytecode_append_mov(
        x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&B.allocation));
    break;
  }

  case OPRFMT_CONSTANT: {
    x64bytecode_append_mov(
        x64bc, x64opr_alloc(&A.allocation), x64opr_constant(I.B));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64bytecode_append_mov(
        x64bc, x64opr_alloc(&A.allocation), x64opr_immediate(I.B));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_neg(Instruction I,
                       u16 Idx,
                       X64Bytecode *restrict x64bc,
                       X64Allocator *restrict allocator) {
  // assert that we don't generate trivially foldable instructions
  assert(I.Bfmt == OPRFMT_SSA);

  X64ActiveAllocation B = *x64allocator_allocation_of(allocator, I.B);
  X64ActiveAllocation A =
      x64allocator_allocate_from_active(allocator, Idx, I.A, &B, x64bc);

  x64bytecode_append_neg(x64bc, x64opr_alloc(&A.allocation));
}

static void x64gen_add(Instruction I,
                       u16 Idx,
                       X64Bytecode *restrict x64bc,
                       X64Allocator *restrict allocator) {
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    X64ActiveAllocation B = *x64allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
      // if B or C is in a gpr we use it as the allocation point of A
      // and the destination operand of the x64 add instruction.
      // this is to try and keep the result, A, in a register.
      if (B.allocation.kind == ALLOC_GPR) {
        X64ActiveAllocation A =
            x64allocator_allocate_from_active(allocator, Idx, I.A, &B, x64bc);
        x64bytecode_append_add(
            x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&C.allocation));
        return;
      }

      if (C.allocation.kind == ALLOC_GPR) {
        X64ActiveAllocation A =
            x64allocator_allocate_from_active(allocator, Idx, I.A, &C, x64bc);
        x64bytecode_append_add(
            x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&B.allocation));
        return;
      }

      // since B and C are memory operands we have to move B or C
      // to a reg and then add.
      X64GPR gpr = x64allocator_spill_oldest_active(allocator, x64bc);
      X64ActiveAllocation A =
          x64allocator_allocate_to_gpr(allocator, gpr, Idx, I.A, x64bc);

      // we use the huristic of longest lifetime to choose
      // which of B and C to move into A's gpr.
      if (B.lifetime.last_use <= C.lifetime.last_use) {
        x64bytecode_append_mov(
            x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&C.allocation));
        x64bytecode_append_add(
            x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&B.allocation));
      } else {
        x64bytecode_append_mov(
            x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&B.allocation));
        x64bytecode_append_add(
            x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&C.allocation));
      }
      break;
    }

    case OPRFMT_CONSTANT: {
      X64ActiveAllocation A =
          x64allocator_allocate_from_active(allocator, Idx, I.A, &B, x64bc);

      x64bytecode_append_add(
          x64bc, x64opr_alloc(&A.allocation), x64opr_constant(I.C));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      X64ActiveAllocation A =
          x64allocator_allocate_from_active(allocator, Idx, I.A, &B, x64bc);

      x64bytecode_append_add(
          x64bc, x64opr_alloc(&A.allocation), x64opr_immediate(I.C));
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(I.Cfmt == OPRFMT_SSA);
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
    X64ActiveAllocation A =
        x64allocator_allocate_from_active(allocator, Idx, I.A, &C, x64bc);

    x64bytecode_append_add(
        x64bc, x64opr_alloc(&A.allocation), x64opr_constant(I.B));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.Cfmt == OPRFMT_SSA);
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
    X64ActiveAllocation A =
        x64allocator_allocate_from_active(allocator, Idx, I.A, &C, x64bc);

    x64bytecode_append_add(
        x64bc, x64opr_alloc(&A.allocation), x64opr_immediate(I.B));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_sub(Instruction I,
                       u16 Idx,
                       X64Bytecode *restrict x64bc,
                       X64Allocator *restrict allocator) {
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    X64ActiveAllocation B = *x64allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
      // since subtraction is not commutative we have to allocate A from B
      // regardless of which of B or C is in a register.
      if ((B.allocation.kind == ALLOC_GPR) ||
          (C.allocation.kind == ALLOC_GPR)) {
        X64ActiveAllocation A =
            x64allocator_allocate_from_active(allocator, Idx, I.A, &B, x64bc);

        x64bytecode_append_sub(
            x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&C.allocation));
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
      X64GPR gpr = x64allocator_aquire_any_gpr(allocator, Idx, x64bc);
      X64ActiveAllocation A =
          x64allocator_allocate_to_gpr(allocator, gpr, Idx, I.A, x64bc);

      x64bytecode_append_mov(
          x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&B.allocation));

      x64bytecode_append_sub(
          x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&C.allocation));
      break;
    }

    case OPRFMT_CONSTANT: {
      X64ActiveAllocation A =
          x64allocator_allocate_from_active(allocator, Idx, I.A, &B, x64bc);

      x64bytecode_append_sub(
          x64bc, x64opr_alloc(&A.allocation), x64opr_constant(I.C));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      X64ActiveAllocation A =
          x64allocator_allocate_from_active(allocator, Idx, I.A, &B, x64bc);

      x64bytecode_append_sub(
          x64bc, x64opr_alloc(&A.allocation), x64opr_immediate(I.C));
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
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);

    X64GPR gpr = x64allocator_aquire_any_gpr(allocator, Idx, x64bc);
    x64bytecode_append_mov(x64bc, x64opr_gpr(gpr), x64opr_constant(I.B));
    X64ActiveAllocation A =
        x64allocator_allocate_to_gpr(allocator, gpr, Idx, I.A, x64bc);

    x64bytecode_append_sub(
        x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&C.allocation));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.Cfmt == OPRFMT_SSA);
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);

    X64GPR gpr = x64allocator_aquire_any_gpr(allocator, Idx, x64bc);
    x64bytecode_append_mov(x64bc, x64opr_gpr(gpr), x64opr_immediate(I.B));
    X64ActiveAllocation A =
        x64allocator_allocate_to_gpr(allocator, gpr, Idx, I.A, x64bc);

    x64bytecode_append_sub(
        x64bc, x64opr_alloc(&A.allocation), x64opr_alloc(&C.allocation));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_mul(Instruction I,
                       u16 Idx,
                       X64Bytecode *restrict x64bc,
                       X64Allocator *restrict allocator) {
  /*
    imul takes a single reg/mem argument,
    and expects the other argument to be in %rax
    and stores the result in %rdx:%rax.
  */

  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    X64ActiveAllocation B = *x64allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
      if ((B.allocation.kind == ALLOC_GPR) &&
          (B.allocation.gpr == X64GPR_RAX)) {
        x64allocator_allocate_from_active(allocator, Idx, I.A, &B, x64bc);

        x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);

        x64bytecode_append_imul(x64bc, x64opr_alloc(&C.allocation));
        break;
      }

      if ((C.allocation.kind == ALLOC_GPR) &&
          (C.allocation.gpr == X64GPR_RAX)) {
        x64allocator_allocate_from_active(allocator, Idx, I.A, &C, x64bc);

        x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);

        x64bytecode_append_imul(x64bc, x64opr_alloc(&B.allocation));
        break;
      }

      x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);
      x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      if ((B.lifetime.last_use <= C.lifetime.last_use)) {
        x64bytecode_append_mov(
            x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));
        x64bytecode_append_imul(x64bc, x64opr_alloc(&C.allocation));
      } else {
        x64bytecode_append_mov(
            x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&C.allocation));
        x64bytecode_append_imul(x64bc, x64opr_alloc(&B.allocation));
      }
      break;
    }

    case OPRFMT_CONSTANT: {
      x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);
      x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RDX), x64opr_constant(I.C));
      x64bytecode_append_imul(x64bc, x64opr_alloc(&B.allocation));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);
      x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(I.C));
      x64bytecode_append_imul(x64bc, x64opr_alloc(&B.allocation));
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(I.Cfmt == OPRFMT_SSA);
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
    x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);
    x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);

    x64bytecode_append_mov(x64bc, x64opr_gpr(X64GPR_RDX), x64opr_constant(I.B));
    x64bytecode_append_imul(x64bc, x64opr_alloc(&C.allocation));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.Cfmt == OPRFMT_SSA);
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
    x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);
    x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);

    x64bytecode_append_mov(
        x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(I.B));
    x64bytecode_append_imul(x64bc, x64opr_alloc(&C.allocation));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_div(Instruction I,
                       u16 Idx,
                       X64Bytecode *restrict x64bc,
                       X64Allocator *restrict allocator) {
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    X64ActiveAllocation B = *x64allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
      if ((B.allocation.kind == ALLOC_GPR) &&
          (B.allocation.gpr == X64GPR_RAX)) {
        x64allocator_allocate_from_active(allocator, Idx, I.A, &B, x64bc);

        x64allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64bytecode_append_mov(
            x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

        x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));
        x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        break;
      }

      if ((C.allocation.kind == ALLOC_GPR) &&
          (C.allocation.gpr == X64GPR_RAX)) {
        x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);

        x64allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64bytecode_append_mov(
            x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

        x64allocator_reallocate_active(allocator, &C, x64bc);

        x64bytecode_append_mov(
            x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));

        x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));
        x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        break;
      }

      x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);

      x64allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));

      x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));
      x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    case OPRFMT_CONSTANT: {
      x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));

      x64allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

      X64GPR gpr = x64allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64bytecode_append_mov(x64bc, x64opr_gpr(gpr), x64opr_constant(I.C));

      x64bytecode_append_idiv(x64bc, x64opr_gpr(gpr));

      x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));

      x64allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

      X64GPR gpr = x64allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64bytecode_append_mov(x64bc, x64opr_gpr(gpr), x64opr_immediate(I.C));

      x64bytecode_append_idiv(x64bc, x64opr_gpr(gpr));

      x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    x64allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    x64bytecode_append_mov(x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
    if ((C.allocation.kind == ALLOC_GPR) && (C.allocation.gpr == X64GPR_RAX)) {
      x64allocator_reallocate_active(allocator, &C, x64bc);
    }

    x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);

    x64bytecode_append_mov(x64bc, x64opr_gpr(X64GPR_RAX), x64opr_constant(I.B));
    x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));

    x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    x64bytecode_append_mov(x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
    if ((C.allocation.kind == ALLOC_GPR) && (C.allocation.gpr == X64GPR_RAX)) {
      x64allocator_reallocate_active(allocator, &C, x64bc);
    }

    x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);

    x64bytecode_append_mov(
        x64bc, x64opr_gpr(X64GPR_RAX), x64opr_immediate(I.B));
    x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));

    x64allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    break;
  }

  default: unreachable();
  }
}

static void x64gen_mod(Instruction I,
                       u16 Idx,
                       X64Bytecode *restrict x64bc,
                       X64Allocator *restrict allocator) {
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    X64ActiveAllocation B = *x64allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
      if ((B.allocation.kind == ALLOC_GPR) &&
          (B.allocation.gpr == X64GPR_RAX)) {
        x64allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, I.A, x64bc);
        x64bytecode_append_mov(
            x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

        x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));
        break;
      }

      if ((C.allocation.kind == ALLOC_GPR) &&
          (C.allocation.gpr == X64GPR_RAX)) {
        x64allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, I.A, x64bc);

        x64allocator_reallocate_active(allocator, &C, x64bc);

        x64allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
        x64bytecode_append_mov(
            x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));

        x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));
        break;
      }

      x64allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, I.A, x64bc);
      x64allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));

      x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));
      break;
    }

    case OPRFMT_CONSTANT: {
      x64allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, I.A, x64bc);
      x64allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));

      X64GPR gpr = x64allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64bytecode_append_mov(x64bc, x64opr_gpr(gpr), x64opr_constant(I.C));

      x64bytecode_append_idiv(x64bc, x64opr_gpr(gpr));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, I.A, x64bc);
      x64allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64bytecode_append_mov(
          x64bc, x64opr_gpr(X64GPR_RAX), x64opr_alloc(&B.allocation));

      X64GPR gpr = x64allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64bytecode_append_mov(x64bc, x64opr_gpr(gpr), x64opr_immediate(I.C));

      x64bytecode_append_idiv(x64bc, x64opr_gpr(gpr));
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    x64allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, I.A, x64bc);
    x64bytecode_append_mov(x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
    if ((C.allocation.kind == ALLOC_GPR) && (C.allocation.gpr == X64GPR_RAX)) {
      x64allocator_reallocate_active(allocator, &C, x64bc);
    }

    x64allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, I.A, x64bc);

    x64bytecode_append_mov(x64bc, x64opr_gpr(X64GPR_RAX), x64opr_constant(I.B));
    x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, I.A, x64bc);
    x64bytecode_append_mov(x64bc, x64opr_gpr(X64GPR_RDX), x64opr_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    X64ActiveAllocation C = *x64allocator_allocation_of(allocator, I.C);
    if ((C.allocation.kind == ALLOC_GPR) && (C.allocation.gpr == X64GPR_RAX)) {
      x64allocator_reallocate_active(allocator, &C, x64bc);
    }

    x64bytecode_append_mov(
        x64bc, x64opr_gpr(X64GPR_RAX), x64opr_immediate(I.B));
    x64bytecode_append_idiv(x64bc, x64opr_alloc(&C.allocation));
    break;
  }

  default: unreachable();
  }
}

static void x64gen_bytecode(Bytecode *restrict bc,
                            X64Bytecode *restrict x64bc,
                            X64Allocator *restrict allocator) {
  for (u16 idx = 0; idx < bc->length; ++idx) {
    Instruction I = bc->buffer[idx];

    switch (I.opcode) {
    case OPC_RET: {
      x64gen_ret(I, x64bc, allocator);
      break;
    }

    case OPC_MOVE: {
      x64gen_move(I, idx, x64bc, allocator);
      break;
    }

    case OPC_NEG: {
      x64gen_neg(I, idx, x64bc, allocator);
      break;
    }

    case OPC_ADD: {
      x64gen_add(I, idx, x64bc, allocator);
      break;
    }

    case OPC_SUB: {
      x64gen_sub(I, idx, x64bc, allocator);
      break;
    }

    case OPC_MUL: {
      x64gen_mul(I, idx, x64bc, allocator);
      break;
    }

    case OPC_DIV: {
      x64gen_div(I, idx, x64bc, allocator);
      break;
    }

    case OPC_MOD: {
      x64gen_mod(I, idx, x64bc, allocator);
      break;
    }

    default: unreachable();
    }
  }

  x64bytecode_prepend_mov(
      x64bc, x64opr_gpr(X64GPR_RBP), x64opr_gpr(X64GPR_RSP));
  x64bytecode_prepend_push(x64bc, x64opr_gpr(X64GPR_RBP));
  if (allocator->stack_size > 0) {
    x64bytecode_prepend_sub(
        x64bc, x64opr_gpr(X64GPR_RSP), x64opr_immediate(allocator->stack_size));
  }
}

static void x64gen_function(FunctionBody *restrict body,
                            X64FunctionBody *restrict x64body) {
  Bytecode *bc           = &body->bc;
  X64Bytecode *x64bc     = &x64body->bc;
  X64Allocator allocator = x64allocator_create(body);

  x64gen_bytecode(bc, x64bc, &allocator);
  x64body->stack_size = allocator.stack_size;

  x64allocator_destroy(&allocator);
}

static void x64gen_ste(SymbolTableElement *restrict ste,
                       X64Context *restrict x64context) {
  StringView name      = ste->name;
  X64Symbol *x64symbol = x64context_symbol(x64context, name);

  switch (ste->kind) {
  case STE_UNDEFINED: {
    // #TODO this should lower to a forward declaration
    break;
  }

  case STE_FUNCTION: {
    FunctionBody *body       = &ste->function_body;
    X64FunctionBody *x64body = &x64symbol->body;
    *x64body                 = x64function_body_create(body->arguments.size);
    x64gen_function(body, x64body);
    break;
  }

  default: unreachable();
  }
}

void x64codegen(Context *restrict context) {
  X64Context x64context = x64context_create(context);

  SymbolTableIterator iter = context_global_symbol_iterator(context);

  while (!symbol_table_iterator_done(&iter)) {
    x64gen_ste(iter.element, &x64context);

    symbol_table_iterator_next(&iter);
  }

  x64emit(&x64context);
  x64context_destroy(&x64context);
}
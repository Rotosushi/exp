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
#include "utility/panic.h"

static void x64_codegen_copy(x64_Location dest,
                             x64_Location source,
                             u16 Idx,
                             x64_Bytecode *restrict x64bc,
                             x64_Allocator *restrict allocator) {
  if ((dest.kind == ALLOC_STACK) && (source.kind == ALLOC_STACK)) {
    x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);

    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(gpr), x64_operand_location(source));
    x64_bytecode_append_mov(
        x64bc, x64_operand_location(dest), x64_operand_gpr(gpr));

    x64_allocator_release_gpr(allocator, gpr, Idx, x64bc);
    return;
  }

  x64_bytecode_append_mov(
      x64bc, x64_operand_location(dest), x64_operand_location(source));
}

static void x64_codegen_ret(Instruction I,
                            u16 Idx,
                            x64_FunctionBody *restrict body,
                            [[maybe_unused]] LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc = &body->bc;
  // since we are returning, we know by definition
  // that lifetimes are ending. thus we can just emit
  // mov's where necessary.
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    if (x64_allocation_location_eq(B, body->return_location)) { break; }
    x64_codegen_copy(body->return_location, B->location, Idx, x64bc, allocator);
    break;
  }

  case OPRFMT_CONSTANT: {
    x64_bytecode_append_mov(x64bc,
                            x64_operand_location(body->return_location),
                            x64_operand_constant(I.B));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_bytecode_append_mov(x64bc,
                            x64_operand_location(body->return_location),
                            x64_operand_immediate(I.B));
    break;
  }

  case OPRFMT_ARGUMENT: {
    x64_FormalArgument *arg =
        x64_formal_argument_list_at(&body->arguments, (u8)I.B);

    x64_codegen_copy(
        body->return_location, arg->location, Idx, x64bc, allocator);
    break;
  }

  case OPRFMT_LABEL: {
    PANIC("#TODO");
    break;
  }

  default: unreachable();
  }

  x64_bytecode_append_mov(
      x64bc, x64_operand_gpr(X64GPR_RSP), x64_operand_gpr(X64GPR_RBP));
  x64_bytecode_append_pop(x64bc, x64_operand_gpr(X64GPR_RBP));
  x64_bytecode_append_ret(x64bc);
}

static void
x64_codegen_actual_argument(Operand arg,
                            [[maybe_unused]] u8 arg_idx,
                            [[maybe_unused]] u16 Idx,
                            [[maybe_unused]] x64_FunctionBody *restrict body,
                            [[maybe_unused]] LocalVariables *restrict locals,
                            [[maybe_unused]] x64_Allocator *restrict allocator,
                            [[maybe_unused]] x64_Context *restrict context) {
  switch (arg.format) {
  case OPRFMT_SSA: {
    break;
  }

  case OPRFMT_CONSTANT: {
    break;
  }

  case OPRFMT_IMMEDIATE: {
    break;
  }

  case OPRFMT_ARGUMENT: {
    break;
  }

  case OPRFMT_LABEL: {
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_call(Instruction I,
                             u16 Idx,
                             x64_FunctionBody *restrict body,
                             LocalVariables *restrict locals,
                             x64_Allocator *restrict allocator,
                             x64_Context *restrict context) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  x64_allocator_allocate_result(allocator, Idx, local, x64bc);

  ActualArgumentList *args = x64_context_call_at(context, I.C);
  for (u8 i = 0; i < args->size; ++i) {
    Operand arg = args->list[i];
    x64_codegen_actual_argument(arg, i, Idx, body, locals, allocator, context);
  }

  x64_bytecode_append_call(x64bc, x64_operand_label(I.B));
}

static void x64_codegen_load(Instruction I,
                             u16 Idx,
                             x64_FunctionBody *restrict body,
                             LocalVariables *restrict locals,
                             x64_Allocator *restrict allocator) {
  /*
    the load instruction can be lowered to
    the x64 mov instruction nearly 1:1
  */
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  x64_Allocation *A    = x64_allocator_allocate(allocator, Idx, local, x64bc);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    x64_codegen_copy(A->location, B->location, Idx, x64bc, allocator);
    break;
  }

  case OPRFMT_CONSTANT: {
    x64_bytecode_append_mov(
        x64bc, x64_operand_alloc(A), x64_operand_constant(I.B));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_bytecode_append_mov(
        x64bc, x64_operand_alloc(A), x64_operand_immediate(I.B));
    break;
  }

  case OPRFMT_ARGUMENT: {
    x64_FormalArgument *arg =
        x64_formal_argument_list_at(&body->arguments, (u8)I.B);
    x64_codegen_copy(A->location, arg->location, Idx, x64bc, allocator);
    break;
  }

  case OPRFMT_LABEL: {
    PANIC("#TODO");
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_neg(Instruction I,
                            u16 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    x64_Allocation *A =
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

    x64_bytecode_append_neg(x64bc, x64_operand_alloc(A));
    break;
  }

  case OPRFMT_CONSTANT:
  case OPRFMT_IMMEDIATE: {
    // assert that we don't generate trivially foldable instructions
    assert(0);
    break;
  }

  case OPRFMT_ARGUMENT: {
    x64_FormalArgument *arg =
        x64_formal_argument_list_at(&body->arguments, (u8)I.B);
    // #NOTE: by definition the argument's lifetime is the whole function.
    // so in order to maintain the value of the argument we initialize a
    // new allocation with the value of the argument in order to negate that.
    x64_Allocation *A = x64_allocator_allocate(allocator, Idx, local, x64bc);
    x64_codegen_copy(A->location, arg->location, Idx, x64bc, allocator);

    x64_bytecode_append_neg(x64bc, x64_operand_alloc(A));
    break;
  }

  case OPRFMT_LABEL: {
    PANIC("#TODO");
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_add(Instruction I,
                            u16 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
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
        x64_bytecode_append_add(
            x64bc, x64_operand_alloc(A), x64_operand_alloc(C));
        return;
      }

      if (C->location.kind == ALLOC_GPR) {
        x64_Allocation *A =
            x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);
        x64_bytecode_append_add(
            x64bc, x64_operand_alloc(A), x64_operand_alloc(B));
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
        x64_bytecode_append_mov(
            x64bc, x64_operand_alloc(A), x64_operand_alloc(C));
        x64_bytecode_append_add(
            x64bc, x64_operand_alloc(A), x64_operand_alloc(B));
      } else {
        x64_bytecode_append_mov(
            x64bc, x64_operand_alloc(A), x64_operand_alloc(B));
        x64_bytecode_append_add(
            x64bc, x64_operand_alloc(A), x64_operand_alloc(C));
      }
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append_add(
          x64bc, x64_operand_alloc(A), x64_operand_constant(I.C));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append_add(
          x64bc, x64_operand_alloc(A), x64_operand_immediate(I.C));
      break;
    }

    case OPRFMT_ARGUMENT: {
      x64_FormalArgument *arg =
          x64_formal_argument_list_at(&body->arguments, (u8)I.C);
      // arguments are const, so we use B as the allocation point of A
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append_add(
          x64bc, x64_operand_alloc(A), x64_operand_location(arg->location));
      break;
    }

    case OPRFMT_LABEL: {
      PANIC("#TODO");
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

    x64_bytecode_append_add(
        x64bc, x64_operand_alloc(A), x64_operand_constant(I.B));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    x64_Allocation *A =
        x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

    x64_bytecode_append_add(
        x64bc, x64_operand_alloc(A), x64_operand_immediate(I.B));
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_sub(Instruction I,
                            u16 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A);
  switch (I.Bfmt) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B);
    switch (I.Cfmt) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
      // #NOTE since subtraction is not commutative we have to allocate A from B
      // regardless of which of B or C is in a register.
      if ((B->location.kind == ALLOC_GPR) || (C->location.kind == ALLOC_GPR)) {
        x64_Allocation *A =
            x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_bytecode_append_sub(
            x64bc, x64_operand_alloc(A), x64_operand_alloc(C));
        return;
      }

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_Allocation *A =
          x64_allocator_allocate_to_gpr(allocator, gpr, Idx, local, x64bc);

      x64_bytecode_append_mov(
          x64bc, x64_operand_alloc(A), x64_operand_alloc(B));

      x64_bytecode_append_sub(
          x64bc, x64_operand_alloc(A), x64_operand_alloc(C));
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append_sub(
          x64bc, x64_operand_alloc(A), x64_operand_constant(I.C));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append_sub(
          x64bc, x64_operand_alloc(A), x64_operand_immediate(I.C));
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
    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(gpr), x64_operand_constant(I.B));
    x64_Allocation *A =
        x64_allocator_allocate_to_gpr(allocator, gpr, Idx, local, x64bc);

    x64_bytecode_append_sub(x64bc, x64_operand_alloc(A), x64_operand_alloc(C));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);

    x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(gpr), x64_operand_immediate(I.B));
    x64_Allocation *A =
        x64_allocator_allocate_to_gpr(allocator, gpr, Idx, local, x64bc);

    x64_bytecode_append_sub(x64bc, x64_operand_alloc(A), x64_operand_alloc(C));
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_mul(Instruction I,
                            u16 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  /*
  #NOTE:
    imul takes a single reg/mem argument,
    and expects the other argument to be in %rax
    and stores the result in %rdx:%rax.
  */
  x64_Bytecode *x64bc  = &body->bc;
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

        x64_bytecode_append_imul(x64bc, x64_operand_alloc(C));
        break;
      }

      if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);

        x64_bytecode_append_imul(x64bc, x64_operand_alloc(B));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      if ((B->lifetime.last_use <= C->lifetime.last_use)) {
        x64_bytecode_append_mov(
            x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B));
        x64_bytecode_append_imul(x64bc, x64_operand_alloc(C));
      } else {
        x64_bytecode_append_mov(
            x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(C));
        x64_bytecode_append_imul(x64bc, x64_operand_alloc(B));
      }
      break;
    }

    case OPRFMT_CONSTANT: {
      if (x64_allocation_location_eq(B, x64_location_reg(X64GPR_RAX))) {
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_constant(I.C));
        x64_bytecode_append_imul(x64bc, x64_operand_alloc(B));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_constant(I.C));
      x64_bytecode_append_imul(x64bc, x64_operand_alloc(B));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      if (x64_allocation_location_eq(B, x64_location_reg(X64GPR_RAX))) {
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(I.C));
        x64_bytecode_append_imul(x64bc, x64_operand_alloc(B));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_immediate(I.C));
      x64_bytecode_append_imul(x64bc, x64_operand_alloc(B));
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
          x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_constant(I.B));
      x64_bytecode_append_imul(x64bc, x64_operand_alloc(C));
      break;
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_constant(I.B));
    x64_bytecode_append_imul(x64bc, x64_operand_alloc(C));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(I.B));
      x64_bytecode_append_imul(x64bc, x64_operand_alloc(C));
      break;
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_immediate(I.B));
    x64_bytecode_append_imul(x64bc, x64_operand_alloc(C));
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_div(Instruction I,
                            u16 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
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
            x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

        x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));
        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        break;
      }

      if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

        x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

        x64_allocator_reallocate_active(allocator, C, x64bc);

        x64_bytecode_append_mov(
            x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B));

        x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));
        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

      x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B));

      x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));
      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B));

      x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(gpr), x64_operand_constant(I.C));

      x64_bytecode_append_idiv(x64bc, x64_operand_gpr(gpr));

      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B));

      x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(gpr), x64_operand_immediate(I.C));

      x64_bytecode_append_idiv(x64bc, x64_operand_gpr(gpr));

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
        x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_constant(I.B));
    x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));

    x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_immediate(I.B));
    x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));

    x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_mod(Instruction I,
                            u16 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
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
            x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

        x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));
        break;
      }

      if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);

        x64_allocator_reallocate_active(allocator, C, x64bc);

        x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
        x64_bytecode_append_mov(
            x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B));

        x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
      x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B));

      x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
      x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(gpr), x64_operand_constant(I.C));

      x64_bytecode_append_idiv(x64bc, x64_operand_gpr(gpr));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
      x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(gpr), x64_operand_immediate(I.C));

      x64_bytecode_append_idiv(x64bc, x64_operand_gpr(gpr));
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_constant(I.B));
    x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0));

    assert(I.Cfmt == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C);
    if ((C->location.kind == ALLOC_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(X64GPR_RAX), x64_operand_immediate(I.B));
    x64_bytecode_append_idiv(x64bc, x64_operand_alloc(C));
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_bytecode(Bytecode *restrict bc,
                                 x64_FunctionBody *restrict body,
                                 LocalVariables *restrict locals,
                                 x64_Allocator *restrict allocator,
                                 x64_Context *restrict context) {
  for (u16 idx = 0; idx < bc->length; ++idx) {
    Instruction I = bc->buffer[idx];

    switch (I.opcode) {
    case OPC_RET: {
      x64_codegen_ret(I, idx, body, locals, allocator);
      break;
    }

    case OPC_CALL: {
      x64_codegen_call(I, idx, body, locals, allocator, context);
      break;
    }

    case OPC_LOAD: {
      x64_codegen_load(I, idx, body, locals, allocator);
      break;
    }

    case OPC_NEG: {
      x64_codegen_neg(I, idx, body, locals, allocator);
      break;
    }

    case OPC_ADD: {
      x64_codegen_add(I, idx, body, locals, allocator);
      break;
    }

    case OPC_SUB: {
      x64_codegen_sub(I, idx, body, locals, allocator);
      break;
    }

    case OPC_MUL: {
      x64_codegen_mul(I, idx, body, locals, allocator);
      break;
    }

    case OPC_DIV: {
      x64_codegen_div(I, idx, body, locals, allocator);
      break;
    }

    case OPC_MOD: {
      x64_codegen_mod(I, idx, body, locals, allocator);
      break;
    }

    default: unreachable();
    }
  }
}

static void x64_codegen_function_header(x64_Allocator *restrict allocator,
                                        x64_FunctionBody *restrict body) {
  x64_Bytecode *x64bc = &body->bc;
  if (x64_allocator_uses_stack(allocator)) {
    x64_bytecode_prepend_sub(
        x64bc,
        x64_operand_gpr(X64GPR_RSP),
        x64_operand_immediate(x64_allocator_total_stack_size(allocator)));
  }
  x64_bytecode_prepend_mov(
      x64bc, x64_operand_gpr(X64GPR_RBP), x64_operand_gpr(X64GPR_RSP));
  x64_bytecode_prepend_push(x64bc, x64_operand_gpr(X64GPR_RBP));
}

static void x64_codegen_function(FunctionBody *restrict body,
                                 x64_FunctionBody *restrict x64_body,
                                 x64_Context *restrict context) {
  LocalVariables *locals  = &body->locals;
  Bytecode *bc            = &body->bc;
  x64_Allocator allocator = x64_allocator_create(body);

  x64_codegen_bytecode(bc, x64_body, locals, &allocator, context);

  x64_body->stack_size = x64_allocator_total_stack_size(&allocator);
  x64_codegen_function_header(&allocator, x64_body);

  x64_allocator_destroy(&allocator);
}

static void x64_codegen_ste(SymbolTableElement *restrict ste,
                            x64_Context *restrict context) {
  StringView name    = ste->name;
  x64_Symbol *symbol = x64_context_symbol(context, name);

  switch (ste->kind) {
  case STE_UNDEFINED: {
    // #TODO this should lower to a forward declaration
    break;
  }

  case STE_FUNCTION: {
    x64_context_enter_function(context, name);
    FunctionBody *body        = &ste->function_body;
    x64_FunctionBody *x64body = &symbol->body;
    *x64body                  = x64_function_body_create(body->arguments.size);
    x64_codegen_function(body, x64body, context);
    x64_context_leave_function(context);
    break;
  }

  default: unreachable();
  }
}

void x64_codegen(Context *restrict context) {
  x64_Context x64context = x64_context_create(context);

  SymbolTableIterator iter = context_global_symbol_table_iterator(context);

  while (!symbol_table_iterator_done(&iter)) {
    x64_codegen_ste(iter.element, &x64context);

    symbol_table_iterator_next(&iter);
  }

  x64_emit(&x64context);
  x64_context_destroy(&x64context);
}
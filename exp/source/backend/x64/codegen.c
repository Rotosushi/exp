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
#include <stddef.h>

#include "backend/x64/allocator.h"
#include "backend/x64/codegen.h"
#include "backend/x64/context.h"
#include "backend/x64/emit.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "utility/minmax.h"
#include "utility/panic.h"

// #TODO we have to copy from one location to the other,
// but we also need type information to handle larger size objects.
// and we have to have a copy for constants, immediates, and labels.

static void x64_codegen_copy_scalar(x64_Allocation *restrict dst,
                                    x64_Allocation *restrict src,
                                    u16 Idx,
                                    x64_Bytecode *restrict x64bc,
                                    x64_Allocator *restrict allocator) {
  if ((dst->location.kind != ALLOC_STACK) ||
      (src->location.kind != ALLOC_STACK)) {
    x64_bytecode_append_mov(
        x64bc, x64_operand_alloc(dst), x64_operand_alloc(src));
    return;
  }

  x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);

  x64_bytecode_append_mov(x64bc, x64_operand_gpr(gpr), x64_operand_alloc(src));
  x64_bytecode_append_mov(x64bc, x64_operand_alloc(dst), x64_operand_gpr(gpr));

  x64_allocator_release_gpr(allocator, gpr, Idx, x64bc);
}

static void x64_codegen_copy_composite(x64_Allocation *restrict dst,
                                       x64_Allocation *restrict src,
                                       u16 Idx,
                                       x64_Bytecode *restrict x64bc,
                                       x64_Allocator *restrict allocator) {
  assert(dst->location.kind == ALLOC_STACK);
  assert(src->location.kind == ALLOC_STACK);
  assert(type_equality(dst->type, src->type));
  x64_GPR gpr   = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
  Type *type    = dst->type;
  u64 type_size = size_of(type);

  // mov src+8*n(%rbp), gpr
  // mov gpr, dst+8*n(%rbp)
  u16 src_offset = src->location.offset;
  u16 dst_offset = dst->location.offset;

  // #NOTE #TODO: as it currently stands, all
  // types are length 8. to make math like this
  // simpler. This is obviously inefficient.
  u16 elements = (u16)(type_size / 8);
  for (u16 i = 0; i < elements; ++i) {
    u16 src_element_offset = src_offset + (8 * i);
    assert(src_element_offset <= i16_MAX);
    u16 dst_element_offset = dst_offset + (8 * i);
    assert(dst_element_offset <= i16_MAX);

    x64_Location src_element = x64_location_stack((i16)src_element_offset);
    x64_Location dst_element = x64_location_stack((i16)dst_element_offset);

    x64_bytecode_append_mov(
        x64bc, x64_operand_gpr(gpr), x64_operand_location(src_element));
    x64_bytecode_append_mov(
        x64bc, x64_operand_location(dst_element), x64_operand_gpr(gpr));
  }

  x64_allocator_release_gpr(allocator, gpr, Idx, x64bc);
}

static void x64_codegen_copy(x64_Allocation *restrict dst,
                             x64_Allocation *restrict src,
                             u16 Idx,
                             x64_Bytecode *restrict x64bc,
                             x64_Allocator *restrict allocator) {
  assert(type_equality(dst->type, src->type));

  if (x64_location_eq(dst->location, src->location)) { return; }

  if (type_is_scalar(dst->type)) {
    x64_codegen_copy_scalar(dst, src, Idx, x64bc, allocator);
  } else {
    x64_codegen_copy_composite(dst, src, Idx, x64bc, allocator);
  }
}

static void
x64_codegen_load_constant_composite(x64_Allocation *restrict dst,
                                    Value *restrict src,
                                    u16 Idx,
                                    x64_Bytecode *restrict x64bc,
                                    x64_Allocator *restrict allocator) {}

static void x64_codegen_load_constant(x64_Allocation *restrict dst,
                                      u16 index,
                                      u16 Idx,
                                      x64_Bytecode *restrict x64bc,
                                      x64_Allocator *restrict allocator,
                                      x64_Context *restrict context) {
  Value *src     = context_constants_at(context->context, index);
  Type *src_type = type_of_value(src, context->context);
  assert(type_equality(dst->type, src_type));

  if (type_is_scalar(dst->type)) {
    x64_bytecode_append_mov(
        x64bc, x64_operand_alloc(dst), x64_operand_constant(index));
  } else {
    x64_codegen_copy_composite(dst, src, Idx, x64bc, allocator);
  }
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

static x64_GPR x64_scalar_argument_gpr(u8 num) {
  // #TODO: for now, all possible types are scalar.
  // so when we introduce types that cannot be passed
  // by register we have to account for that here.
  // all scalar arguments are passed as below,
  // any non-scalar argument goes straight to the stack
  // no matter which index it is. and thus we cannot
  // rely on the index to be 1:1 with the register being
  // selected.
  switch (num) {
  case 0: return X64GPR_RDI;
  case 1: return X64GPR_RSI;
  case 2: return X64GPR_RDX;
  case 3: return X64GPR_RCX;
  case 4: return X64GPR_R8;
  case 5: return X64GPR_R9;
  // the rest of the arguments are passed on the stack.
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
  x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

  ActualArgumentList *args    = x64_context_call_at(context, I.C);
  u16 current_bytecode_offset = x64_bytecode_current_offset(x64bc);
  u8 scalar_argument_count    = 0;
  u8 i                        = 0;

  for (; (i < args->size) && (i < 6); ++i) {
    Operand arg = args->list[i];
    x64_GPR gpr = x64_scalar_argument_gpr(scalar_argument_count++);
    x64_allocator_release_gpr(allocator, gpr, Idx, x64bc);

    switch (arg.format) {
    case OPRFMT_SSA: {
      LocalVariable *local = local_variables_lookup_ssa(locals, arg.common);
      x64_Allocation *allocation =
          x64_allocator_allocation_of(allocator, local->ssa);

      x64_codegen_copy(
          x64_location_gpr(gpr), allocation->location, Idx, x64bc, allocator);
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(gpr), x64_operand_constant(arg.common));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(gpr), x64_operand_immediate(arg.common));
      break;
    }

    case OPRFMT_LABEL: {
      x64_bytecode_append_mov(
          x64bc, x64_operand_gpr(gpr), x64_operand_label(arg.common));
      break;
    }

    default: unreachable();
    }
  }

  // if we initialized all of the arguments, all we have left
  // is to emit the call and return.
  if (i >= args->size) {
    x64_bytecode_append_call(x64bc, x64_operand_label(I.B));
    return;
  }

  // initialize the stack passed arguments, then we can emit the
  // call and return.
  u16 actual_arguments_stack_size = 0;
  u16 current_stack_offset        = x64_allocator_total_stack_size(allocator);
  // reverse iterate the stack passed arguments, so we push them
  // onto the stack in the correct order.
  for (u8 j = args->size - 1; j >= i; --j) {
    Operand arg = args->list[j];
    Type *type  = type_of_operand(arg, context->context);

    u16 arg_size = (u16)ulmax(8UL, size_of(type));
    u16 offset =
        (u16)(current_stack_offset + actual_arguments_stack_size + arg_size);
    assert(offset <= i16_MAX);
    i16 arg_offset = -((i16)offset);

    actual_arguments_stack_size += arg_size;

    switch (arg.format) {
    case OPRFMT_SSA: {
      LocalVariable *local = local_variables_lookup_ssa(locals, arg.common);
      x64_Allocation *allocation =
          x64_allocator_allocation_of(allocator, local->ssa);

      x64_codegen_copy(x64_location_stack(arg_offset),
                       allocation->location,
                       Idx,
                       x64bc,
                       allocator);
      break;
    }

    case OPRFMT_CONSTANT: {
      x64_bytecode_append_mov(x64bc,
                              x64_operand_stack(arg_offset),
                              x64_operand_constant(arg.common));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_bytecode_append_mov(x64bc,
                              x64_operand_stack(arg_offset),
                              x64_operand_immediate(arg.common));
      break;
    }

    case OPRFMT_LABEL: {
      x64_bytecode_append_mov(
          x64bc, x64_operand_stack(arg_offset), x64_operand_label(arg.common));
      break;
    }

    default: unreachable();
    }
  }

  x64_bytecode_insert_sub(x64bc,
                          current_bytecode_offset,
                          x64_operand_gpr(X64GPR_RSP),
                          x64_operand_immediate(actual_arguments_stack_size));

  x64_bytecode_append_call(x64bc, x64_operand_label(I.B));

  x64_bytecode_append_add(x64bc,
                          x64_operand_gpr(X64GPR_RSP),
                          x64_operand_immediate(actual_arguments_stack_size));
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

  case OPRFMT_LABEL: {
    PANIC("#TODO");
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
      if (x64_allocation_location_eq(B, x64_location_gpr(X64GPR_RAX))) {
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
      if (x64_allocation_location_eq(B, x64_location_gpr(X64GPR_RAX))) {
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
    if (x64_allocation_location_eq(C, x64_location_gpr(X64GPR_RAX))) {
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
  LocalVariables *locals   = &body->locals;
  Bytecode *bc             = &body->bc;
  FormalArgumentList *args = &body->arguments;
  x64_Allocator allocator  = x64_allocator_create(body);
  x64_Bytecode *x64bc      = &x64_body->bc;

  // allocate the incoming arguments to the function.
  // first we allocate the incoming GPR arguments.
  u8 i = 0;
  for (; (i < args->size) && (i < 6); ++i) {
    FormalArgument *arg  = args->list + i;
    LocalVariable *local = local_variables_lookup_ssa(locals, arg->ssa);
    x64_GPR gpr          = x64_scalar_argument_gpr(i);
    x64_allocator_allocate_to_gpr(&allocator, gpr, 0, local, x64bc);
  }

  // then if the rest of the incoming arguments are passed on the stack
  // pushed from right-to-left, this means that the first stack passed
  // argument is on the stack immediately above the pushed %rbp.
  // so we just have to increment a number, to select each successive
  // stack passed argument.
  if (i < args->size) {
    // the initial offset is 8, to skip the pushed %rbp
    i16 arg_offset = 8;
    for (; i < args->size; ++i) {
      FormalArgument *arg  = args->list + i;
      LocalVariable *local = local_variables_lookup_ssa(locals, arg->ssa);
      u16 offset           = (u16)ulmax(8UL, size_of(arg->type));
      assert(offset <= i16_MAX);
      arg_offset += (i16)offset;
      x64_allocator_allocate_formal_argument_to_stack(
          &allocator, arg_offset, local);
    }
  }

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

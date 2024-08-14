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
#include "backend/x64/intrinsics/copy.h"
#include "backend/x64/intrinsics/load.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/minmax.h"
#include "utility/panic.h"

static void x64_codegen_ret(Instruction I,
                            u64 Idx,
                            x64_FunctionBody *restrict body,
                            [[maybe_unused]] LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator,
                            x64_Context *restrict context) {
  x64_Bytecode *x64bc = &body->bc;
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B.ssa);
    if (x64_allocation_location_eq(B, body->result->location)) { break; }
    x64_codegen_copy_allocation(body->result, B, Idx, x64bc, allocator);
    break;
  }

  case OPRFMT_CONSTANT: {
    x64_codegen_load_allocation(
        body->result, I.B.index, Idx, x64bc, allocator, context);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_bytecode_append(x64bc,
                        x64_mov(x64_operand_alloc(body->result),
                                x64_operand_immediate(I.B.immediate)));
    break;
  }

  case OPRFMT_LABEL: {
    /*
     * #NOTE #TODO #FEATURE eventually we will add support for
     * global constants (global variables are in limbo until
     * proven vital). When these exist, it will be possible to
     * access them via OPRFMT_LABEL operands. Since we do not
     * have them yet, this case is effecively unreachable.
     */
    PANIC("#TODO");
    break;
  }

  default: unreachable();
  }

  x64_bytecode_append(
      x64bc, x64_mov(x64_operand_gpr(X64GPR_RSP), x64_operand_gpr(X64GPR_RBP)));
  x64_bytecode_append(x64bc, x64_pop(x64_operand_gpr(X64GPR_RBP)));
  x64_bytecode_append(x64bc, x64_ret());
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

typedef struct OperandArray {
  u8 size;
  u8 capacity;
  Operand *buffer;
} OperandArray;

static OperandArray operand_array_create() {
  OperandArray array = {.size = 0, .capacity = 0, .buffer = nullptr};
  return array;
}

static void operand_array_destroy(OperandArray *restrict array) {
  deallocate(array->buffer);
  array->size     = 0;
  array->capacity = 0;
  array->buffer   = nullptr;
}

static bool operand_array_full(OperandArray *restrict array) {
  return (array->size + 1) >= array->capacity;
}

static void operand_array_grow(OperandArray *restrict array) {
  Growth g        = array_growth_u8(array->capacity, sizeof(Operand));
  array->buffer   = reallocate(array->buffer, g.alloc_size);
  array->capacity = (u8)g.new_capacity;
}

static void operand_array_append(OperandArray *restrict array,
                                 Operand operand) {
  if (operand_array_full(array)) { operand_array_grow(array); }
  array->buffer[array->size++] = operand;
}

static void
x64_codegen_load_scalar_argument(x64_GPR dst,
                                 Operand *arg,
                                 u64 Idx,
                                 x64_Bytecode *restrict x64bc,
                                 x64_Allocator *restrict allocator) {
  x64_allocator_release_gpr(allocator, dst, Idx, x64bc);
  switch (arg->format) {
  case OPRFMT_SSA: {
    x64_Allocation *allocation =
        x64_allocator_allocation_of(allocator, arg->ssa);
    x64_bytecode_append(
        x64bc, x64_mov(x64_operand_gpr(dst), x64_operand_alloc(allocation)));
    break;
  }

  case OPRFMT_CONSTANT: {
    /*
     * #NOTE we do not generate scalar constants anymore as immediate
     * has been widened to an i64. However, This will need to handle
     * arguments of scalar type which are not i64's. For now, we do not
     * handle types other than i64.
     */
    PANIC("unreachable");
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_bytecode_append(
        x64bc,
        x64_mov(x64_operand_gpr(dst), x64_operand_immediate(arg->immediate)));
    break;
  }

  case OPRFMT_LABEL: {
    PANIC("#TODO");
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_call(Instruction I,
                             u64 Idx,
                             x64_FunctionBody *restrict body,
                             LocalVariables *restrict locals,
                             x64_Allocator *restrict allocator,
                             x64_Context *restrict context) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A.ssa);
  x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

  ActualArgumentList *args    = x64_context_call_at(context, I.C.index);
  u64 current_bytecode_offset = x64_bytecode_current_offset(x64bc);
  u8 scalar_argument_count    = 0;
  OperandArray stack_args     = operand_array_create();

  for (u8 i = 0; i < args->size; ++i) {
    Operand *arg   = args->list + i;
    Type *arg_type = type_of_operand(arg, context->context);

    if (type_is_scalar(arg_type) && (scalar_argument_count < 6)) {
      x64_GPR gpr = x64_scalar_argument_gpr(scalar_argument_count++);
      x64_codegen_load_scalar_argument(gpr, arg, Idx, x64bc, allocator);
    } else {
      operand_array_append(&stack_args, *arg);
    }
  }

  if (stack_args.size == 0) {
    x64_bytecode_append(x64bc, x64_call(x64_operand_label(I.B.index)));
    return;
  }

  i64 current_stack_offset        = -x64_allocator_total_stack_size(allocator);
  i64 actual_arguments_stack_size = 0;

  x64_Address arg_address =
      x64_address_construct(X64GPR_RBP,
                            x64_optional_gpr_empty(),
                            x64_optional_u8_empty(),
                            x64_optional_i64(current_stack_offset));

  for (u8 i = stack_args.size; i > 0; --i) {
    Operand *arg   = stack_args.buffer + (i - 1);
    Type *arg_type = type_of_operand(arg, context->context);
    u64 arg_size   = size_of(arg_type);

    assert(arg_size <= i64_MAX);
    i64 offset = -((i64)(arg_size));
    actual_arguments_stack_size += -offset;
    x64_address_increment_offset(&arg_address, offset);

    x64_codegen_load_from_operand(
        &arg_address, arg, arg_type, Idx, x64bc, allocator, context);
  }

  x64_bytecode_insert(
      x64bc,
      x64_sub(x64_operand_gpr(X64GPR_RSP),
              x64_operand_immediate(actual_arguments_stack_size)),
      current_bytecode_offset);

  x64_bytecode_append(x64bc, x64_call(x64_operand_label(I.B.index)));

  x64_bytecode_append(
      x64bc,
      x64_add(x64_operand_gpr(X64GPR_RSP),
              x64_operand_immediate(actual_arguments_stack_size)));

  operand_array_destroy(&stack_args);
}

static void x64_codegen_load(Instruction I,
                             u64 Idx,
                             x64_FunctionBody *restrict body,
                             LocalVariables *restrict locals,
                             x64_Allocator *restrict allocator,
                             x64_Context *restrict context) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A.ssa);
  x64_Allocation *A    = x64_allocator_allocate(allocator, Idx, local, x64bc);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B.ssa);
    x64_codegen_copy_allocation(A, B, Idx, x64bc, allocator);
    break;
  }

  case OPRFMT_CONSTANT: {
    x64_codegen_load_allocation(A, I.B.index, Idx, x64bc, allocator, context);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_bytecode_append(
        x64bc,
        x64_mov(x64_operand_alloc(A), x64_operand_immediate(I.B.immediate)));
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
                            u64 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A.ssa);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B.ssa);
    x64_Allocation *A =
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

    x64_bytecode_append(x64bc, x64_neg(x64_operand_alloc(A)));
    break;
  }

  case OPRFMT_CONSTANT:
  case OPRFMT_IMMEDIATE: {
    // assert that we don't generate trivially foldable instructions
    PANIC("unreachable");
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
                            u64 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A.ssa);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B.ssa);
    switch (I.C.format) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);
      // if B or C is in a gpr we use it as the allocation point of A
      // and the destination operand of the x64 add instruction.
      // this is to try and keep the result, A, in a register.
      if (B->location.kind == LOCATION_GPR) {
        x64_Allocation *A =
            x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);
        x64_bytecode_append(
            x64bc, x64_add(x64_operand_alloc(A), x64_operand_alloc(C)));
        return;
      }

      if (C->location.kind == LOCATION_GPR) {
        x64_Allocation *A =
            x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);
        x64_bytecode_append(
            x64bc, x64_add(x64_operand_alloc(A), x64_operand_alloc(B)));
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
        x64_bytecode_append(
            x64bc, x64_mov(x64_operand_alloc(A), x64_operand_alloc(C)));
        x64_bytecode_append(
            x64bc, x64_add(x64_operand_alloc(A), x64_operand_alloc(B)));
      } else {
        x64_bytecode_append(
            x64bc, x64_mov(x64_operand_alloc(A), x64_operand_alloc(B)));
        x64_bytecode_append(
            x64bc, x64_add(x64_operand_alloc(A), x64_operand_alloc(C)));
      }
      break;
    }

    case OPRFMT_CONSTANT: {
      // #NOTE we do not add types which are not i64 (yet), thus there is no
      // way to generate an instruction which adds to a constant. (floating
      // point types are the exception to this.)
      PANIC("unreachable");
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append(
          x64bc,
          x64_add(x64_operand_alloc(A), x64_operand_immediate(I.C.immediate)));
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
    PANIC("unreachable");
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.C.format == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);
    x64_Allocation *A =
        x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

    x64_bytecode_append(
        x64bc,
        x64_add(x64_operand_alloc(A), x64_operand_immediate(I.B.immediate)));
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
                            u64 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A.ssa);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B.ssa);
    switch (I.C.format) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);
      // #NOTE since subtraction is not commutative we have to allocate A from B
      // regardless of which of B or C is in a register.
      if ((B->location.kind == LOCATION_GPR) ||
          (C->location.kind == LOCATION_GPR)) {
        x64_Allocation *A =
            x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_bytecode_append(
            x64bc, x64_sub(x64_operand_alloc(A), x64_operand_alloc(C)));
        return;
      }

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_Allocation *A =
          x64_allocator_allocate_to_gpr(allocator, gpr, Idx, local, x64bc);

      x64_bytecode_append(x64bc,
                          x64_mov(x64_operand_alloc(A), x64_operand_alloc(B)));

      x64_bytecode_append(x64bc,
                          x64_sub(x64_operand_alloc(A), x64_operand_alloc(C)));
      break;
    }

    case OPRFMT_CONSTANT: {
      PANIC("unreachable");
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_Allocation *A =
          x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

      x64_bytecode_append(
          x64bc,
          x64_sub(x64_operand_alloc(A), x64_operand_immediate(I.C.immediate)));
      break;
    }

    default: unreachable();
    }
    break;
  }
  case OPRFMT_CONSTANT: {
    PANIC("unreachable");
    break;
  }

  /*
   * #NOTE: there is no x64 sub instruction which takes an
   * immediate value on the lhs. so we have to move the
   * value of B into a gpr and allocate A there.
   * Then we can emit the sub instruction.
   *
   * #NOTE that we could also aquire a stack slot for the immediate
   * value, and emit the sub. this is a single instruction if and
   * only if C was in a gpr; otherwise we are forced to emit a mov
   * to ensure one operand of the sub is in a gpr.
   * and, as above, this leaves the result in memory,
   * which makes it more likely that a future usage will result
   * in generating another mov instruction.
   *
   * I don't know the difference in timings for the various forms
   * of the sub instruction, my assumption is (reg,reg) is faster than
   * either (reg/mem) or (mem/reg); and relatively equal to (reg,imm).
   */
  case OPRFMT_IMMEDIATE: {
    assert(I.C.format == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);

    x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
    x64_bytecode_append(
        x64bc,
        x64_mov(x64_operand_gpr(gpr), x64_operand_immediate(I.B.immediate)));
    x64_Allocation *A =
        x64_allocator_allocate_to_gpr(allocator, gpr, Idx, local, x64bc);

    x64_bytecode_append(x64bc,
                        x64_sub(x64_operand_alloc(A), x64_operand_alloc(C)));
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_mul(Instruction I,
                            u64 Idx,
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
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A.ssa);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B.ssa);
    switch (I.C.format) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);
      if ((B->location.kind == LOCATION_GPR) &&
          (B->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);

        x64_bytecode_append(x64bc, x64_imul(x64_operand_alloc(C)));
        break;
      }

      if ((C->location.kind == LOCATION_GPR) &&
          (C->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);

        x64_bytecode_append(x64bc, x64_imul(x64_operand_alloc(B)));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      if ((B->lifetime.last_use <= C->lifetime.last_use)) {
        x64_bytecode_append(
            x64bc, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));
        x64_bytecode_append(x64bc, x64_imul(x64_operand_alloc(C)));
      } else {
        x64_bytecode_append(
            x64bc, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(C)));
        x64_bytecode_append(x64bc, x64_imul(x64_operand_alloc(B)));
      }
      break;
    }

    case OPRFMT_CONSTANT: {
      PANIC("unreachable");
      break;
    }

    case OPRFMT_IMMEDIATE: {
      if (x64_allocation_location_eq(B, x64_location_gpr(X64GPR_RAX))) {
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append(x64bc,
                            x64_mov(x64_operand_gpr(X64GPR_RDX),
                                    x64_operand_immediate(I.C.immediate)));
        x64_bytecode_append(x64bc, x64_imul(x64_operand_alloc(B)));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append(x64bc,
                          x64_mov(x64_operand_gpr(X64GPR_RAX),
                                  x64_operand_immediate(I.C.immediate)));
      x64_bytecode_append(x64bc, x64_imul(x64_operand_alloc(B)));
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    PANIC("unreachable");
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.C.format == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);
    if ((C->location.kind == LOCATION_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_allocate_from_active(allocator, Idx, local, C, x64bc);

      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append(x64bc,
                          x64_mov(x64_operand_gpr(X64GPR_RDX),
                                  x64_operand_immediate(I.B.immediate)));
      x64_bytecode_append(x64bc, x64_imul(x64_operand_alloc(C)));
      break;
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
    x64_bytecode_append(x64bc,
                        x64_mov(x64_operand_gpr(X64GPR_RAX),
                                x64_operand_immediate(I.B.immediate)));
    x64_bytecode_append(x64bc, x64_imul(x64_operand_alloc(C)));
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_div(Instruction I,
                            u64 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A.ssa);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B.ssa);
    switch (I.C.format) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);
      if ((B->location.kind == LOCATION_GPR) &&
          (B->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_from_active(allocator, Idx, local, B, x64bc);

        x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append(
            x64bc,
            x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

        x64_bytecode_append(x64bc, x64_idiv(x64_operand_alloc(C)));
        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        break;
      }

      if ((C->location.kind == LOCATION_GPR) &&
          (C->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

        x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        x64_bytecode_append(
            x64bc,
            x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

        x64_allocator_reallocate_active(allocator, C, x64bc);

        x64_bytecode_append(
            x64bc, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

        x64_bytecode_append(x64bc, x64_idiv(x64_operand_alloc(C)));
        x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

      x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append(
          x64bc,
          x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

      x64_bytecode_append(
          x64bc, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

      x64_bytecode_append(x64bc, x64_idiv(x64_operand_alloc(C)));
      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    case OPRFMT_CONSTANT: {
      PANIC("unreachable");
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);
      x64_bytecode_append(
          x64bc, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

      x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      x64_bytecode_append(
          x64bc,
          x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append(
          x64bc,
          x64_mov(x64_operand_gpr(gpr), x64_operand_immediate(I.C.immediate)));

      x64_bytecode_append(x64bc, x64_idiv(x64_operand_gpr(gpr)));

      x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    PANIC("unreachable");
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.C.format == OPRFMT_SSA);
    x64_allocator_aquire_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    x64_bytecode_append(
        x64bc, x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);
    if ((C->location.kind == LOCATION_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_allocator_allocate_to_gpr(allocator, X64GPR_RAX, Idx, local, x64bc);

    x64_bytecode_append(x64bc,
                        x64_mov(x64_operand_gpr(X64GPR_RAX),
                                x64_operand_immediate(I.B.immediate)));
    x64_bytecode_append(x64bc, x64_idiv(x64_operand_alloc(C)));

    x64_allocator_release_gpr(allocator, X64GPR_RDX, Idx, x64bc);
    break;
  }

  default: unreachable();
  }
}

static void x64_codegen_mod(Instruction I,
                            u64 Idx,
                            x64_FunctionBody *restrict body,
                            LocalVariables *restrict locals,
                            x64_Allocator *restrict allocator) {
  x64_Bytecode *x64bc  = &body->bc;
  LocalVariable *local = local_variables_lookup_ssa(locals, I.A.ssa);
  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_allocator_allocation_of(allocator, I.B.ssa);
    switch (I.C.format) {
    case OPRFMT_SSA: {
      x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);
      if ((B->location.kind == LOCATION_GPR) &&
          (B->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
        x64_bytecode_append(
            x64bc,
            x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

        x64_bytecode_append(x64bc, x64_idiv(x64_operand_alloc(C)));
        break;
      }

      if ((C->location.kind == LOCATION_GPR) &&
          (C->location.gpr == X64GPR_RAX)) {
        x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);

        x64_allocator_reallocate_active(allocator, C, x64bc);

        x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
        x64_bytecode_append(
            x64bc, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

        x64_bytecode_append(x64bc, x64_idiv(x64_operand_alloc(C)));
        break;
      }

      x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
      x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64_bytecode_append(
          x64bc, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

      x64_bytecode_append(x64bc, x64_idiv(x64_operand_alloc(C)));
      break;
    }

    case OPRFMT_CONSTANT: {
      PANIC("unreachable");
      break;
    }

    case OPRFMT_IMMEDIATE: {
      x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
      x64_allocator_aquire_gpr(allocator, X64GPR_RAX, Idx, x64bc);
      x64_bytecode_append(
          x64bc, x64_mov(x64_operand_gpr(X64GPR_RAX), x64_operand_alloc(B)));

      x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
      x64_bytecode_append(
          x64bc,
          x64_mov(x64_operand_gpr(gpr), x64_operand_immediate(I.C.immediate)));

      x64_bytecode_append(x64bc, x64_idiv(x64_operand_gpr(gpr)));
      break;
    }

    default: unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    PANIC("unreachable");
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_allocator_allocate_to_gpr(allocator, X64GPR_RDX, Idx, local, x64bc);
    x64_bytecode_append(
        x64bc, x64_mov(x64_operand_gpr(X64GPR_RDX), x64_operand_immediate(0)));

    assert(I.C.format == OPRFMT_SSA);
    x64_Allocation *C = x64_allocator_allocation_of(allocator, I.C.ssa);
    if ((C->location.kind == LOCATION_GPR) && (C->location.gpr == X64GPR_RAX)) {
      x64_allocator_reallocate_active(allocator, C, x64bc);
    }

    x64_bytecode_append(x64bc,
                        x64_mov(x64_operand_gpr(X64GPR_RAX),
                                x64_operand_immediate(I.B.immediate)));
    x64_bytecode_append(x64bc, x64_idiv(x64_operand_alloc(C)));
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
  for (u64 idx = 0; idx < bc->length; ++idx) {
    Instruction I = bc->buffer[idx];

    switch (I.opcode) {
    case OPC_RET: {
      x64_codegen_ret(I, idx, body, locals, allocator, context);
      break;
    }

    case OPC_CALL: {
      x64_codegen_call(I, idx, body, locals, allocator, context);
      break;
    }

    case OPC_LOAD: {
      x64_codegen_load(I, idx, body, locals, allocator, context);
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
    x64_bytecode_prepend(
        x64bc,
        x64_sub(
            x64_operand_gpr(X64GPR_RSP),
            x64_operand_immediate(x64_allocator_total_stack_size(allocator))));
  }
  x64_bytecode_prepend(
      x64bc, x64_mov(x64_operand_gpr(X64GPR_RBP), x64_operand_gpr(X64GPR_RSP)));
  x64_bytecode_prepend(x64bc, x64_push(x64_operand_gpr(X64GPR_RBP)));
}

static void x64_codegen_function(FunctionBody *restrict body,
                                 x64_FunctionBody *restrict x64_body,
                                 x64_Context *restrict context) {
  LocalVariables *locals   = &body->locals;
  Bytecode *bc             = &body->bc;
  FormalArgumentList *args = &body->arguments;
  x64_Allocator allocator  = x64_allocator_create(body);
  x64_Bytecode *x64bc      = &x64_body->bc;

  if (type_is_scalar(body->return_type)) {
    x64_body->result = x64_allocator_allocate_result(
        &allocator, x64_location_gpr(X64GPR_RAX), body->return_type);
  } else {
    x64_body->result = x64_allocator_allocate_result(
        &allocator,
        x64_location_address(X64GPR_RDI,
                             x64_optional_gpr_empty(),
                             x64_optional_u8_empty(),
                             x64_optional_i64_empty()),
        body->return_type);
  }

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
      x64_allocator_allocate_to_stack(&allocator, arg_offset, local);
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

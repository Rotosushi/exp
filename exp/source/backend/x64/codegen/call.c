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

#include "backend/x64/codegen/call.h"
#include "backend/x64/intrinsics/load.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

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

void x64_codegen_call(Instruction I, u64 Idx, x64_Context *restrict context) {
  LocalVariable *local     = x64_context_lookup_ssa(context, I.A.ssa);
  u8 scalar_argument_count = 0;

  if (type_is_scalar(local->type)) {
    x64_context_allocate_to_gpr(context, local, X64GPR_RAX, Idx);
  } else {
    x64_Allocation *result = x64_context_allocate(context, local, Idx);
    assert(result->location.kind == LOCATION_ADDRESS);
    x64_context_append(context,
                       x64_lea(x64_operand_gpr(x64_scalar_argument_gpr(
                                   scalar_argument_count++)),
                               x64_operand_address(result->location.address)));
  }

  assert(I.C_format == OPRFMT_CALL);
  ActualArgumentList *args    = x64_context_call_at(context, I.C.index);
  u64 current_bytecode_offset = x64_context_current_offset(context);
  OperandArray stack_args     = operand_array_create();

  for (u8 i = 0; i < args->size; ++i) {
    Operand arg    = args->list[i];
    Type *arg_type = type_of_operand(arg.format, arg.value, context->context);

    if (type_is_scalar(arg_type) && (scalar_argument_count < 6)) {
      x64_GPR gpr = x64_scalar_argument_gpr(scalar_argument_count++);
      x64_codegen_load_gpr_from_operand(
          gpr, arg.format, arg.value, Idx, context);
    } else {
      operand_array_append(&stack_args, arg);
    }
  }

  assert(I.B_format == OPRFMT_LABEL);
  if (stack_args.size == 0) {
    x64_context_append(context, x64_call(x64_operand_label(I.B.index)));
    return;
  }

  i64 actual_arguments_stack_size = 0;
  x64_Address arg_address         = x64_address_from_gpr(X64GPR_RSP);

  for (u8 i = 0; i < stack_args.size; ++i) {
    Operand arg    = stack_args.buffer[i];
    Type *arg_type = type_of_operand(arg.format, arg.value, context->context);
    u64 arg_size   = size_of(arg_type);
    assert(arg_size <= i64_MAX);
    i64 offset = (i64)(arg_size);
    actual_arguments_stack_size += offset;

    x64_codegen_load_address_from_operand(
        &arg_address, arg.format, arg.value, arg_type, Idx, context);

    x64_address_increment_offset(&arg_address, offset);
  }

  x64_context_insert(
      context,
      x64_sub(x64_operand_gpr(X64GPR_RSP),
              x64_operand_immediate(actual_arguments_stack_size)),
      current_bytecode_offset);

  x64_context_append(context, x64_call(x64_operand_label(I.B.index)));

  x64_context_append(
      context,
      x64_add(x64_operand_gpr(X64GPR_RSP),
              x64_operand_immediate(actual_arguments_stack_size)));

  operand_array_destroy(&stack_args);
}

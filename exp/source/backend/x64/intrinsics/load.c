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

#include "backend/x64/intrinsics/address_of.h"
#include "backend/x64/intrinsics/copy.h"
#include "backend/x64/intrinsics/load.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "utility/unreachable.h"

static void
x64_codegen_load_address_from_scalar_value(x64_Address *restrict dst,
                                           Value *restrict value,
                                           x64_Context *restrict context) {
  switch (value->kind) {
  case VALUEKIND_UNINITIALIZED: break; // don't initialize the uninitialized

  case VALUEKIND_NIL: {
    x64_context_append(
        context, x64_mov(x64_operand_address(*dst), x64_operand_immediate(0)));
    break;
  }

  case VALUEKIND_BOOLEAN: {
    x64_context_append(context,
                       x64_mov(x64_operand_address(*dst),
                               x64_operand_immediate((i64)value->boolean)));
    break;
  }

  case VALUEKIND_I64: {
    x64_context_append(context,
                       x64_mov(x64_operand_address(*dst),
                               x64_operand_immediate(value->integer_64)));
    break;
  }

  case VALUEKIND_TUPLE:
  default:              EXP_UNREACHABLE;
  }
}

static void x64_codegen_load_address_from_scalar_operand(
    x64_Address *restrict dst,
    OperandFormat format,
    OperandValue operand,
    [[maybe_unused]] Type *restrict type,
    u64 Idx,
    x64_Context *restrict context) {
  assert(type_is_scalar(type));

  switch (format) {
  case OPRFMT_SSA: {
    x64_Allocation *allocation =
        x64_context_allocation_of(context, operand.ssa);
    if (allocation->location.kind == LOCATION_GPR) {
      x64_context_append(context,
                         x64_mov(x64_operand_address(*dst),
                                 x64_operand_gpr(allocation->location.gpr)));
    } else {
      x64_codegen_copy_scalar_memory(
          dst, &allocation->location.address, Idx, context);
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_context_append(context,
                       x64_mov(x64_operand_address(*dst),
                               x64_operand_immediate(operand.immediate)));
    break;
  }

  case OPRFMT_VALUE: {
    Value *value = x64_context_value_at(context, operand.index);
    assert(type_equality(type, type_of_value(value, context->context)));
    x64_codegen_load_address_from_scalar_value(dst, value, context);
    break;
  }

  case OPRFMT_LABEL: {
    x64_Address global = x64_address_of_global(operand.index, Idx, context);
    x64_codegen_copy_memory(dst,
                            &global,
                            type_of_operand(format, operand, context->context),
                            Idx,
                            context);
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

static void
x64_codegen_load_address_from_composite_operand(x64_Address *restrict dst,
                                                OperandFormat format,
                                                OperandValue operand,
                                                Type *restrict type,
                                                u64 Idx,
                                                x64_Context *restrict context) {
  switch (format) {
  case OPRFMT_SSA: {
    x64_Allocation *allocation =
        x64_context_allocation_of(context, operand.ssa);

    assert(allocation->location.kind == LOCATION_ADDRESS);

    x64_codegen_copy_composite_memory(
        dst, &allocation->location.address, type, Idx, context);
    break;
  }

  case OPRFMT_VALUE: {
    Value *value = x64_context_value_at(context, operand.index);
    Type *type   = type_of_value(value, context->context);
    assert(value->kind == VALUEKIND_TUPLE);
    assert(!type_is_scalar(type));
    Tuple *tuple = &value->tuple;

    x64_Address dst_element_address = *dst;
    for (u64 i = 0; i < tuple->size; ++i) {
      Operand element = tuple->elements[i];
      Type *element_type =
          type_of_operand(element.format, element.value, context->context);
      u64 element_size = size_of(element_type);

      x64_codegen_load_address_from_operand(&dst_element_address,
                                            element.format,
                                            element.value,
                                            element_type,
                                            Idx,
                                            context);

      assert(element_size <= i64_MAX);
      i64 offset = (i64)element_size;
      x64_address_increment_offset(&dst_element_address, offset);
    }

    break;
  }

  case OPRFMT_LABEL: {
    x64_Address global = x64_address_of_global(operand.index, Idx, context);
    x64_codegen_copy_memory(dst,
                            &global,
                            type_of_operand(format, operand, context->context),
                            Idx,
                            context);
    break;
  }

  case OPRFMT_IMMEDIATE:
  default:               EXP_UNREACHABLE;
  }
}

void x64_codegen_load_address_from_operand(x64_Address *restrict dst,
                                           OperandFormat format,
                                           OperandValue operand,
                                           Type *restrict type,
                                           u64 Idx,
                                           x64_Context *restrict context) {
  if (type_is_scalar(type)) {
    x64_codegen_load_address_from_scalar_operand(
        dst, format, operand, type, Idx, context);
  } else {
    x64_codegen_load_address_from_composite_operand(
        dst, format, operand, type, Idx, context);
  }
}

static void x64_codegen_load_argument_from_scalar_operand(
    x64_Address *restrict dst,
    OperandFormat format,
    OperandValue operand,
    [[maybe_unused]] Type *restrict type,
    u64 Idx,
    x64_Context *restrict context) {
  switch (format) {
  case OPRFMT_SSA: {
    x64_Allocation *allocation =
        x64_context_allocation_of(context, operand.ssa);
    if (allocation->location.kind == LOCATION_GPR) {
      x64_context_append(context,
                         x64_mov(x64_operand_address(*dst),
                                 x64_operand_gpr(allocation->location.gpr)));
    } else {
      x64_codegen_copy_scalar_memory(
          dst, &allocation->location.address, Idx, context);
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_context_append(context,
                       x64_mov(x64_operand_address(*dst),
                               x64_operand_immediate(operand.immediate)));
    break;
  }

  case OPRFMT_LABEL: {
    x64_Address global = x64_address_of_global(operand.index, Idx, context);
    x64_codegen_copy_memory(dst,
                            &global,
                            type_of_operand(format, operand, context->context),
                            Idx,
                            context);
    break;
  }

  case OPRFMT_VALUE:
  default:           EXP_UNREACHABLE;
  }
}

static void x64_codegen_load_argument_from_composite_operand(
    x64_Address *restrict dst,
    OperandFormat format,
    OperandValue operand,
    Type *restrict type,
    u64 Idx,
    x64_Context *restrict context) {
  switch (format) {
  case OPRFMT_SSA: {
    x64_Allocation *allocation =
        x64_context_allocation_of(context, operand.ssa);
    assert(allocation->location.kind == LOCATION_ADDRESS);
    x64_codegen_copy_composite_memory(
        dst, &allocation->location.address, type, Idx, context);
    break;
  }

  case OPRFMT_VALUE: {
    Value *value = x64_context_value_at(context, operand.index);
    Type *type   = type_of_value(value, context->context);
    assert(value->kind == VALUEKIND_TUPLE);
    assert(!type_is_scalar(type));
    Tuple *tuple = &value->tuple;

    x64_Address dst_element_address = *dst;
    for (u64 i = 0; i < tuple->size; ++i) {
      Operand element = tuple->elements[i];
      Type *element_type =
          type_of_operand(element.format, element.value, context->context);
      u64 element_size = size_of(element_type);

      x64_codegen_load_argument_from_operand(&dst_element_address,
                                             element.format,
                                             element.value,
                                             element_type,
                                             Idx,
                                             context);

      assert(element_size <= i64_MAX);
      i64 offset = -(i64)element_size;
      x64_address_increment_offset(&dst_element_address, offset);
    }

    break;
  }

  case OPRFMT_LABEL: {
    x64_Address global = x64_address_of_global(operand.index, Idx, context);
    x64_codegen_copy_memory(dst,
                            &global,
                            type_of_operand(format, operand, context->context),
                            Idx,
                            context);
    break;
  }

  case OPRFMT_IMMEDIATE:
  default:               EXP_UNREACHABLE;
  }
}

void x64_codegen_load_argument_from_operand(x64_Address *restrict dst,
                                            OperandFormat format,
                                            OperandValue operand,
                                            Type *restrict type,
                                            u64 Idx,
                                            x64_Context *restrict context) {
  if (type_is_scalar(type)) {
    x64_codegen_load_argument_from_scalar_operand(
        dst, format, operand, type, Idx, context);
  } else {
    x64_codegen_load_argument_from_composite_operand(
        dst, format, operand, type, Idx, context);
  }
}

void x64_codegen_load_gpr_from_operand(x64_GPR gpr,
                                       OperandFormat format,
                                       OperandValue operand,
                                       [[maybe_unused]] u64 Idx,
                                       x64_Context *restrict context) {
  switch (format) {
  case OPRFMT_SSA: {
    x64_Allocation *allocation =
        x64_context_allocation_of(context, operand.ssa);
    x64_context_append(
        context, x64_mov(x64_operand_gpr(gpr), x64_operand_alloc(allocation)));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_context_append(context,
                       x64_mov(x64_operand_gpr(gpr),
                               x64_operand_immediate(operand.immediate)));
    break;
  }

  case OPRFMT_LABEL: {
    x64_Address global = x64_address_of_global(operand.index, Idx, context);
    x64_context_append(
        context, x64_mov(x64_operand_gpr(gpr), x64_operand_address(global)));
    break;
  }

  // we don't create scalar values (yet)
  case OPRFMT_VALUE:
  default:           EXP_UNREACHABLE;
  }
}

void x64_codegen_load_allocation_from_operand(x64_Allocation *restrict dst,
                                              OperandFormat format,
                                              OperandValue operand,
                                              u64 Idx,
                                              x64_Context *restrict context) {
  if (dst->location.kind == LOCATION_ADDRESS) {
    x64_codegen_load_address_from_operand(
        &dst->location.address, format, operand, dst->type, Idx, context);
  } else {
    x64_codegen_load_gpr_from_operand(
        dst->location.gpr, format, operand, Idx, context);
  }
}

void x64_codegen_load_allocation_from_value(x64_Allocation *restrict dst,
                                            u64 index,
                                            u64 Idx,
                                            x64_Context *restrict context) {
  Value *value = x64_context_value_at(context, index);
  Type *type   = type_of_value(value, context->context);

  assert(type_equality(dst->type, type));

  switch (value->kind) {
  case VALUEKIND_UNINITIALIZED: break;

  case VALUEKIND_NIL: {
    x64_context_append(
        context, x64_mov(x64_operand_alloc(dst), x64_operand_immediate(0)));
    break;
  }

  case VALUEKIND_BOOLEAN: {
    x64_context_append(context,
                       x64_mov(x64_operand_alloc(dst),
                               x64_operand_immediate((i64)value->boolean)));
    break;
  }

  case VALUEKIND_I64: {
    x64_context_append(context,
                       x64_mov(x64_operand_alloc(dst),
                               x64_operand_immediate(value->integer_64)));
    break;
  }

  case VALUEKIND_TUPLE: {
    assert(dst->location.kind == LOCATION_ADDRESS);
    Tuple *tuple            = &value->tuple;
    x64_Address dst_address = dst->location.address;
    for (u64 i = 0; i < tuple->size; ++i) {
      Operand element = tuple->elements[i];
      Type *element_type =
          type_of_operand(element.format, element.value, context->context);
      u64 element_size = size_of(element_type);

      x64_codegen_load_address_from_operand(&dst_address,
                                            element.format,
                                            element.value,
                                            element_type,
                                            Idx,
                                            context);

      assert(element_size <= i64_MAX);
      i64 offset = (i64)element_size;
      x64_address_increment_offset(&dst_address, offset);
    }
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

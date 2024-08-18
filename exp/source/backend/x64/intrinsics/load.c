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

#include "backend/x64/intrinsics/copy.h"
#include "backend/x64/intrinsics/load.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "utility/panic.h"
#include "utility/unreachable.h"

static void x64_codegen_load_address_from_scalar_operand(
    x64_Address *restrict dst,
    Operand *restrict src,
    [[maybe_unused]] Type *restrict type,
    u64 Idx,
    x64_Bytecode *restrict x64bc,
    x64_Allocator *restrict allocator) {
  switch (src->format) {
  case OPRFMT_SSA: {
    x64_Allocation *allocation =
        x64_allocator_allocation_of(allocator, src->ssa);
    if (allocation->location.kind == LOCATION_GPR) {
      x64_bytecode_append(x64bc,
                          x64_mov(x64_operand_address(*dst),
                                  x64_operand_gpr(allocation->location.gpr)));
    } else {
      x64_codegen_copy_scalar_memory(
          dst, &allocation->location.address, Idx, x64bc, allocator);
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_bytecode_append(x64bc,
                        x64_mov(x64_operand_address(*dst),
                                x64_operand_immediate(src->immediate)));
    break;
  }

  case OPRFMT_LABEL: {
    PANIC("#TODO");
    break;
  }

  case OPRFMT_VALUE:
  default:           EXP_UNREACHABLE;
  }
}

static void x64_codegen_load_address_from_composite_operand(
    x64_Address *restrict dst,
    Operand *restrict src,
    Type *restrict type,
    u64 Idx,
    x64_Bytecode *restrict x64bc,
    x64_Allocator *restrict allocator,
    x64_Context *restrict context) {
  switch (src->format) {
  case OPRFMT_SSA: {
    x64_Allocation *allocation =
        x64_allocator_allocation_of(allocator, src->ssa);

    assert(allocation->location.kind == LOCATION_ADDRESS);

    x64_codegen_copy_composite_memory(
        dst, &allocation->location.address, type, Idx, x64bc, allocator);
    break;
  }

  case OPRFMT_VALUE: {
    Value *value = x64_context_value_at(context, src->index);
    Type *type   = type_of_value(value, context->context);
    assert(value->kind == VALUEKIND_TUPLE);
    assert(!type_is_scalar(type));
    Tuple *tuple = &value->tuple;

    x64_Address dst_element_address = *dst;
    for (u64 i = 0; i < tuple->size; ++i) {
      Operand *element   = tuple->elements + i;
      Type *element_type = type_of_operand(element, context->context);
      u64 element_size   = size_of(element_type);

      x64_codegen_load_address_from_operand(&dst_element_address,
                                            element,
                                            element_type,
                                            Idx,
                                            x64bc,
                                            allocator,
                                            context);

      assert(element_size <= i64_MAX);
      i64 offset = (i64)element_size;
      x64_address_increment_offset(&dst_element_address, offset);
    }

    break;
  }

  case OPRFMT_LABEL: {
    PANIC("#TODO");
    break;
  }

  case OPRFMT_IMMEDIATE:
  default:               EXP_UNREACHABLE;
  }
}

void x64_codegen_load_address_from_operand(x64_Address *restrict dst,
                                           Operand *restrict src,
                                           Type *restrict type,
                                           u64 Idx,
                                           x64_Bytecode *restrict x64bc,
                                           x64_Allocator *restrict allocator,
                                           x64_Context *restrict context) {
  if (type_is_scalar(type)) {
    x64_codegen_load_address_from_scalar_operand(
        dst, src, type, Idx, x64bc, allocator);
  } else {
    x64_codegen_load_address_from_composite_operand(
        dst, src, type, Idx, x64bc, allocator, context);
  }
}

void x64_codegen_load_gpr_from_operand(
    x64_GPR gpr,
    Operand *restrict src,
    [[maybe_unused]] u64 Idx,
    x64_Bytecode *restrict x64bc,
    x64_Allocator *restrict allocator,
    [[maybe_unused]] x64_Context *restrict context) {
  switch (src->format) {
  case OPRFMT_SSA: {
    x64_Allocation *allocation =
        x64_allocator_allocation_of(allocator, src->ssa);
    x64_bytecode_append(
        x64bc, x64_mov(x64_operand_gpr(gpr), x64_operand_alloc(allocation)));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x64_bytecode_append(
        x64bc,
        x64_mov(x64_operand_gpr(gpr), x64_operand_immediate(src->immediate)));
    break;
  }

  // we don't create scalar values (yet)
  case OPRFMT_VALUE:
  // we don't create globals that are not functions (yet)
  case OPRFMT_LABEL:
  default:           EXP_UNREACHABLE;
  }
}

void x64_codegen_load_allocation_from_operand(x64_Allocation *restrict dst,
                                              Operand *restrict src,
                                              u64 Idx,
                                              x64_Bytecode *restrict x64bc,
                                              x64_Allocator *restrict allocator,
                                              x64_Context *restrict context) {
  if (dst->location.kind == LOCATION_ADDRESS) {
    x64_codegen_load_address_from_operand(
        &dst->location.address, src, dst->type, Idx, x64bc, allocator, context);
  } else {
    x64_codegen_load_gpr_from_operand(
        dst->location.gpr, src, Idx, x64bc, allocator, context);
  }
}

void x64_codegen_load_allocation_from_value(x64_Allocation *restrict dst,
                                            u64 index,
                                            u64 Idx,
                                            x64_Bytecode *restrict x64bc,
                                            x64_Allocator *restrict allocator,
                                            x64_Context *restrict context) {
  Value *value = x64_context_value_at(context, index);
  Type *type   = type_of_value(value, context->context);

  assert(type_equality(dst->type, type));

  switch (value->kind) {
  case VALUEKIND_UNINITIALIZED: break;

  case VALUEKIND_NIL: {
    x64_bytecode_append(
        x64bc, x64_mov(x64_operand_alloc(dst), x64_operand_immediate(0)));
    break;
  }

  case VALUEKIND_BOOLEAN: {
    x64_bytecode_append(x64bc,
                        x64_mov(x64_operand_alloc(dst),
                                x64_operand_immediate((i64)value->boolean)));
    break;
  }

  case VALUEKIND_I64: {
    x64_bytecode_append(x64bc,
                        x64_mov(x64_operand_alloc(dst),
                                x64_operand_immediate(value->integer_64)));
    break;
  }

  case VALUEKIND_TUPLE: {
    assert(dst->location.kind == LOCATION_ADDRESS);
    Tuple *tuple            = &value->tuple;
    x64_Address dst_address = dst->location.address;
    for (u64 i = 0; i < tuple->size; ++i) {
      Operand *element   = tuple->elements + i;
      Type *element_type = type_of_operand(element, context->context);
      u64 element_size   = size_of(element_type);

      x64_codegen_load_address_from_operand(
          &dst_address, element, element_type, Idx, x64bc, allocator, context);

      assert(element_size <= i64_MAX);
      i64 offset = (i64)element_size;
      x64_address_increment_offset(&dst_address, offset);
    }
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

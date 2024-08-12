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

#include "backend/x64/allocator.h"
#include "backend/x64/bytecode.h"
#include "backend/x64/intrinsics/copy.h"
#include "backend/x64/location.h"
#include "imr/type.h"
#include "intrinsics/size_of.h"

void x64_codegen_copy_scalar_memory(x64_Address *restrict dst,
                                    x64_Address *restrict src,
                                    u64 Idx,
                                    x64_Bytecode *restrict x64bc,
                                    x64_Allocator *restrict allocator) {
  x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);

  x64_bytecode_append(x64bc,
                      x64_mov(x64_operand_gpr(gpr), x64_operand_address(*src)));
  x64_bytecode_append(x64bc,
                      x64_mov(x64_operand_address(*dst), x64_operand_gpr(gpr)));

  x64_allocator_release_gpr(allocator, gpr, Idx, x64bc);
}

void x64_codegen_copy_composite_memory(x64_Address *restrict dst,
                                       x64_Address *restrict src,
                                       Type *type,
                                       u64 Idx,
                                       x64_Bytecode *restrict x64bc,
                                       x64_Allocator *restrict allocator) {
  assert(type->kind == TYPEKIND_TUPLE);
  TupleType *tuple_type = &type->tuple_type;

  x64_Address dst_element_address = *dst;
  x64_Address src_element_address = *src;
  for (u64 i = 0; i < tuple_type->size; ++i) {
    Type *element_type = tuple_type->types[i];
    u64 element_size   = size_of(element_type);

    if (type_is_scalar(element_type)) {
      x64_codegen_copy_scalar_memory(
          &dst_element_address, &src_element_address, Idx, x64bc, allocator);
    } else {
      x64_codegen_copy_composite_memory(&dst_element_address,
                                        &src_element_address,
                                        element_type,
                                        Idx,
                                        x64bc,
                                        allocator);
    }

    assert(element_size <= i64_MAX);
    i64 offset = (i64)element_size;
    x64_address_increment_offset(&dst_element_address, offset);
    x64_address_increment_offset(&src_element_address, offset);
  }
}

static void
x64_codegen_copy_scalar_allocation(x64_Allocation *restrict dst,
                                   x64_Allocation *restrict src,
                                   u64 Idx,
                                   x64_Bytecode *restrict x64bc,
                                   x64_Allocator *restrict allocator) {
  if ((dst->location.kind == LOCATION_GPR) ||
      (src->location.kind == LOCATION_GPR)) {
    x64_bytecode_append(
        x64bc, x64_mov(x64_operand_alloc(dst), x64_operand_alloc(src)));
  } else {
    x64_codegen_copy_scalar_memory(
        &dst->location.address, &src->location.address, Idx, x64bc, allocator);
  }
}

void x64_codegen_copy_allocation(x64_Allocation *restrict dst,
                                 x64_Allocation *restrict src,
                                 u64 Idx,
                                 x64_Bytecode *restrict x64bc,
                                 x64_Allocator *restrict allocator) {
  assert(type_equality(dst->type, src->type));

  if (x64_location_eq(dst->location, src->location)) { return; }

  if (type_is_scalar(dst->type)) {
    x64_codegen_copy_scalar_allocation(dst, src, Idx, x64bc, allocator);
  } else {
    assert(dst->location.kind == LOCATION_ADDRESS);
    assert(src->location.kind == LOCATION_ADDRESS);

    x64_codegen_copy_composite_memory(&dst->location.address,
                                      &src->location.address,
                                      dst->type,
                                      Idx,
                                      x64bc,
                                      allocator);
  }
}

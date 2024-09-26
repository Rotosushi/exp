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

#include "backend/x64/codegen/dot.h"
#include "backend/x64/intrinsics/copy.h"
#include "backend/x64/intrinsics/get_element_address.h"
#include "backend/x64/intrinsics/load.h"
#include "intrinsics/type_of.h"
#include "utility/unreachable.h"

void x64_codegen_dot(Instruction I, u64 Idx, x64_Context *restrict context) {
  LocalVariable *local = x64_context_lookup_ssa(context, I.A.ssa);

  assert(I.C.format == OPRFMT_IMMEDIATE);
  assert((I.C.immediate >= 0) && (I.C.immediate <= i64_MAX));
  u64 index         = (u64)I.C.immediate;
  x64_Allocation *A = x64_context_allocate(context, local, Idx);

  switch (I.B.format) {
  case OPRFMT_SSA: {
    x64_Allocation *B = x64_context_allocation_of(context, I.B.ssa);
    assert(B->location.kind == LOCATION_ADDRESS);
    assert(B->type->kind == TYPEKIND_TUPLE);
    x64_Address *tuple_address = &B->location.address;
    x64_Address element_address =
        x64_get_element_address(tuple_address, B->type, index);
    TupleType *tuple_type = &B->type->tuple_type;
    Type *element_type    = tuple_type->types[index];

    x64_codegen_copy_allocation_from_memory(
        A, &element_address, element_type, Idx, context);
    break;
  }

  case OPRFMT_VALUE: {
    x64_codegen_load_allocation_from_value(A, I.B.index, Idx, context);
    break;
  }

  case OPRFMT_LABEL: {
    x64_Address label = x64_address_from_label(I.B.index);
    Type *label_type  = type_of_label(I.B.index, context->context);
    assert(label_type->kind == TYPEKIND_TUPLE);
    TupleType *tuple = &label_type->tuple_type;
    assert(tuple->size > index);
    Type *element_type  = tuple->types[index];
    x64_Address element = x64_get_element_address(&label, label_type, index);
    x64_codegen_copy_allocation_from_memory(
        A, &element, element_type, Idx, context);
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

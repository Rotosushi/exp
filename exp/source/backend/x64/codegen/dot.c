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

/* #NOTE:
 *  There are a few considerations of the dot operation that
 *  have yet to be accounted for.
 *  - most pressing is that we cannot properly address tuples
 *    which reside in global scope. This is because we use the
 *    addressing form (label) to access globals. This form is
 *    not accepted by GAS when we try to access relative to it.
 *    that is 8(label) is not considered a valid address.
 *    The proper way to do this is to load a register with a
 *    pointer to the global.
 *    lea <gpr>, (label)
 *    then we can use the gpr in the addressing form
 *    8(<gpr>)
 *    16(<gpr>)
 *    or anything else.
 *    The question then becomes when do we generate the lea?
 *
 *    optimally we want to generate one lea right before we need it,
 *    only in functions which access the tuple.
 *    This sort of rules out generating it in x64_codegen_dot, because
 *    then we naievely generate a lea per dot. we could guard generation
 *    only if we are accessing a global tuple.
 *    Thats all well and good, but if we create an lea that is essentially
 *    another allocation, another local variable which is unaccounted.
 *    I think we want to move generating the lea instruction into the
 *    bytecode generation. That is, move it to the parser, and implement
 *    global tuple names as pointers.
 */

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
    // x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);

    Type *label_type = type_of_label(I.B.index, context->context);
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

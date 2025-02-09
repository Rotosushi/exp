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
#include "utility/unreachable.h"

void x64_codegen_dot(Instruction I, u64 block_index, x64_Context *context) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = x64_context_lookup_ssa(context, I.A_data.ssa);

    assert(I.C_kind == OPERAND_KIND_I32);
    assert(I.C_data.i32_ >= 0);
    u32 index = (u32)I.C_data.i32_;

    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        x64_Allocation *B = x64_context_allocation_of(context, I.B_data.ssa);
        assert(B->location.kind == LOCATION_ADDRESS);
        assert(B->type->kind == TYPE_KIND_TUPLE);
        x64_Address element_address =
            x64_get_element_address(B->location.address, B->type, index);
        TupleType const *tuple_type = &B->type->tuple_type;
        Type const *element_type    = tuple_type->types[index];

        x64_codegen_copy_allocation_from_memory(
            A, element_address, element_type, block_index, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x64_Allocation *A = x64_context_allocate(context, local, block_index);
        Value *value =
            context_constants_at(context->context, I.B_data.constant);
        assert(value->kind == VALUE_KIND_TUPLE);
        Tuple *tuple = &value->tuple;
        assert(index < tuple->size);
        Operand element = tuple->elements[index];

        x64_codegen_load_allocation_from_operand(
            A, element, block_index, context);
        break;
    }

    // we will never store tuples as immediates
    case OPERAND_KIND_I32:
    // we don't support globals which are not functions yet
    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

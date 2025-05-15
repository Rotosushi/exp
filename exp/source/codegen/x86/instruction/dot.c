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

#include "codegen/x86/instruction/dot.h"
#include "codegen/x86/intrinsics/copy.h"
#include "codegen/x86/intrinsics/get_element_address.h"
#include "codegen/x86/intrinsics/load.h"
#include "support/unreachable.h"

void x86_codegen_dot(Instruction I,
                     u64         block_index,
                     x86_Context *restrict context) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local *local = I.A_data.ssa;

    assert(I.C_kind == OPERAND_KIND_I64);
    assert(I.C_data.i64_ >= 0);
    u16 index = (u16)I.C_data.i64_;

    switch (I.B_kind) {
    case OPERAND_KIND_SSA: {
        x86_Allocation *A = x86_context_allocate(context, local, block_index);
        x86_Allocation *B = x86_context_allocation_of(context, I.B_data.ssa);
        assert(B->location.kind == X86_LOCATION_ADDRESS);
        assert(B->type->kind == TYPE_KIND_TUPLE);
        x86_Address *tuple_address = &B->location.address;
        x86_Address  element_address =
            x86_get_element_address(tuple_address, B->type, index);
        TupleType const *tuple_type   = &B->type->tuple_type;
        Type const      *element_type = tuple_type->types[index];

        x86_codegen_copy_allocation_from_memory(
            A, &element_address, element_type, block_index, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x86_Allocation *A = x86_context_allocate(context, local, block_index);
        Value const    *value = I.B_data.constant;
        assert(value->kind == VALUE_KIND_TUPLE);
        Tuple const *tuple = &value->tuple;
        assert(index < tuple->length);
        Operand operand = tuple->elements[index];
        x86_codegen_load_allocation_from_operand(
            A, operand, block_index, context);
        break;
    }

    // we cannot store tuples as immediates
    case OPERAND_KIND_I64:
    // we don't support globals which are not functions yet
    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

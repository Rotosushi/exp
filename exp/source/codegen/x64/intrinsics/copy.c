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

#include "codegen/x64/imr/location.h"
#include "codegen/x64/intrinsics/copy.h"
#include "imr/type.h"
#include "intrinsics/size_of.h"

void x64_codegen_copy_scalar_memory(x64_Address *restrict dst,
                                    x64_Address *restrict src,
                                    u64 Idx,
                                    x64_Context *restrict context) {
    x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);

    x64_context_append(
        context, x64_mov(x64_operand_gpr(gpr), x64_operand_address(*src)));
    x64_context_append(
        context, x64_mov(x64_operand_address(*dst), x64_operand_gpr(gpr)));

    x64_context_release_gpr(context, gpr, Idx);
}

void x64_codegen_copy_composite_memory(x64_Address *restrict dst,
                                       x64_Address *restrict src,
                                       Type *type,
                                       u64 Idx,
                                       x64_Context *restrict context) {
    assert(type->kind == TYPE_KIND_TUPLE);
    TupleType *tuple_type = &type->tuple_type;

    x64_Address dst_element_address = *dst;
    x64_Address src_element_address = *src;
    for (u64 i = 0; i < tuple_type->size; ++i) {
        Type *element_type = tuple_type->types[i];
        u64 element_size   = size_of(element_type);

        if (type_is_scalar(element_type)) {
            x64_codegen_copy_scalar_memory(
                &dst_element_address, &src_element_address, Idx, context);
        } else {
            x64_codegen_copy_composite_memory(&dst_element_address,
                                              &src_element_address,
                                              element_type,
                                              Idx,
                                              context);
        }

        assert(element_size <= i64_MAX);
        i64 offset = (i64)element_size;
        dst_element_address.offset += offset;
        src_element_address.offset += offset;
    }
}

void x64_codegen_copy_memory(x64_Address *restrict dst,
                             x64_Address *restrict src,
                             Type *type,
                             u64 Idx,
                             x64_Context *restrict context) {
    if (type_is_scalar(type)) {
        x64_codegen_copy_scalar_memory(dst, src, Idx, context);
    } else {
        x64_codegen_copy_composite_memory(dst, src, type, Idx, context);
    }
}

void x64_codegen_copy_allocation_from_memory(x64_Allocation *restrict dst,
                                             x64_Address *restrict src,
                                             Type *restrict type,
                                             u64 Idx,
                                             x64_Context *restrict context) {
    if (dst->location.kind == LOCATION_ADDRESS) {
        x64_codegen_copy_memory(
            &dst->location.address, src, type, Idx, context);
    } else {
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(dst->location.gpr),
                                   x64_operand_address(*src)));
    }
}

static void x64_codegen_copy_scalar_allocation(x64_Allocation *restrict dst,
                                               x64_Allocation *restrict src,
                                               u64 Idx,
                                               x64_Context *restrict context) {
    if ((dst->location.kind == LOCATION_GPR) ||
        (src->location.kind == LOCATION_GPR)) {
        x64_context_append(
            context, x64_mov(x64_operand_alloc(dst), x64_operand_alloc(src)));
    } else {
        x64_codegen_copy_scalar_memory(
            &dst->location.address, &src->location.address, Idx, context);
    }
}

void x64_codegen_copy_allocation(x64_Allocation *restrict dst,
                                 x64_Allocation *restrict src,
                                 u64 Idx,
                                 x64_Context *restrict context) {
    assert(type_equality(dst->type, src->type));

    if (x64_location_eq(dst->location, src->location)) { return; }

    if (type_is_scalar(dst->type)) {
        x64_codegen_copy_scalar_allocation(dst, src, Idx, context);
    } else {
        assert(dst->location.kind == LOCATION_ADDRESS);
        assert(src->location.kind == LOCATION_ADDRESS);

        x64_codegen_copy_composite_memory(&dst->location.address,
                                          &src->location.address,
                                          dst->type,
                                          Idx,
                                          context);
    }
}

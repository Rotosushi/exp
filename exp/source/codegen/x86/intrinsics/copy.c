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

#include "codegen/x86/imr/location.h"
#include "codegen/x86/imr/registers.h"
#include "codegen/x86/intrinsics/copy.h"
#include "imr/type.h"
#include "intrinsics/size_of.h"
#include "support/assert.h"
#include "support/message.h"

void x86_codegen_copy_scalar_memory(x86_Address *restrict dst,
                                    x86_Address *restrict src,
                                    u64 size,
                                    u64 Idx,
                                    x86_Context *restrict context) {
    exp_assert(x86_gpr_valid_size(size));
    x86_GPR gpr = x86_context_aquire_any_gpr(context, size, Idx);

    x86_context_append(
        context, x86_mov(x86_operand_gpr(gpr), x86_operand_address(*src)));
    x86_context_append(
        context, x86_mov(x86_operand_address(*dst), x86_operand_gpr(gpr)));

    x86_context_release_gpr(context, gpr, Idx);
}

void x86_codegen_copy_composite_memory(x86_Address *restrict dst,
                                       x86_Address *restrict src,
                                       Type const *type,
                                       u64         Idx,
                                       x86_Context *restrict context) {
    assert(type->kind == TYPE_KIND_TUPLE);
    TupleType const *tuple_type = &type->tuple;

    x86_Address dst_element_address = *dst;
    x86_Address src_element_address = *src;
    for (u64 i = 0; i < tuple_type->length; ++i) {
        Type const *element_type = tuple_type->types[i];
        u64         element_size = size_of(element_type);

        if (type_is_scalar(element_type)) {
            x86_codegen_copy_scalar_memory(&dst_element_address,
                                           &src_element_address,
                                           element_size,
                                           Idx,
                                           context);
        } else {
            x86_codegen_copy_composite_memory(&dst_element_address,
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

void x86_codegen_copy_memory(x86_Address *restrict dst,
                             x86_Address *restrict src,
                             Type const *type,
                             u64         Idx,
                             x86_Context *restrict context) {
    if (type_is_scalar(type)) {
        u64 size = size_of(type);
        x86_codegen_copy_scalar_memory(dst, src, size, Idx, context);
    } else {
        x86_codegen_copy_composite_memory(dst, src, type, Idx, context);
    }
}

void x86_codegen_copy_allocation_from_memory(x86_Allocation *restrict dst,
                                             x86_Address *restrict src,
                                             Type const *restrict type,
                                             u64 Idx,
                                             x86_Context *restrict context) {
    if (dst->location.kind == X86_LOCATION_ADDRESS) {
        x86_codegen_copy_memory(
            &dst->location.address, src, type, Idx, context);
    } else {
        x86_context_append(context,
                           x86_mov(x86_operand_gpr(dst->location.gpr),
                                   x86_operand_address(*src)));
    }
}

static void x86_codegen_copy_scalar_allocation(x86_Allocation *restrict dst,
                                               x86_Allocation *restrict src,
                                               u64 Idx,
                                               x86_Context *restrict context) {
    if ((dst->location.kind == X86_LOCATION_GPR) ||
        (src->location.kind == X86_LOCATION_GPR)) {
        x86_context_append(
            context, x86_mov(x86_operand_alloc(dst), x86_operand_alloc(src)));
    } else {
        u64 size = size_of(dst->type);
        x86_codegen_copy_scalar_memory(
            &dst->location.address, &src->location.address, size, Idx, context);
    }
}

void x86_codegen_copy_allocation(x86_Allocation *restrict dst,
                                 x86_Allocation *restrict src,
                                 u64 Idx,
                                 x86_Context *restrict context) {
    assert(type_equality(dst->type, src->type));

    if (x86_location_equality(dst->location, src->location)) { return; }

    if (type_is_scalar(dst->type)) {
        x86_codegen_copy_scalar_allocation(dst, src, Idx, context);
    } else {
        assert(dst->location.kind == X86_LOCATION_ADDRESS);
        assert(src->location.kind == X86_LOCATION_ADDRESS);

        x86_codegen_copy_composite_memory(&dst->location.address,
                                          &src->location.address,
                                          dst->type,
                                          Idx,
                                          context);
    }
}

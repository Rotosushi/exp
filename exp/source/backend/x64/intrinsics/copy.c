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

#include "backend/x64/intrinsics/copy.h"
#include "backend/x64/location.h"
#include "imr/type.h"
#include "intrinsics/size_of.h"

void x64_codegen_copy_scalar_memory(u16 target,
                                    u16 source,
                                    u64 block_index,
                                    x64_Context *context) {
    /*
     * #NOTE: so the issue with the "aquire_gpr" api in the allocator
     *  is that it does not truly "aquire" the gpr. In that, if we use
     *  the allocator's other api there is no preservation of the value
     *  held within the GPR. Thus it is possible for codegen to overwrite
     *  the GPR when it's value is still needed at a later point in the block.
     */
    x64_GPR gpr = x64_context_aquire_any_gpr(context, block_index);

    x64_context_append(
        context, x64_mov(x64_operand_gpr(gpr), x64_operand_address(source)));
    x64_context_append(
        context, x64_mov(x64_operand_address(target), x64_operand_gpr(gpr)));

    x64_context_release_gpr(context, gpr, block_index);
}

void x64_codegen_copy_composite_memory(u16 target,
                                       u16 source,
                                       Type const *type,
                                       u64 block_index,
                                       x64_Context *context) {
    assert(type->kind == TYPE_KIND_TUPLE);
    TupleType const *tuple_type = &type->tuple_type;

    for (u64 i = 0; i < tuple_type->count; ++i) {
        Type const *element_type = tuple_type->types[i];
        u64 element_size         = size_of(element_type);

        if (type_is_scalar(element_type)) {
            x64_codegen_copy_scalar_memory(
                target, source, block_index, context);
        } else {
            x64_codegen_copy_composite_memory(
                target, source, element_type, block_index, context);
        }

        x64_Address target_element_address =
            *x64_context_addresses_at(context, target);
        x64_Address source_element_address =
            *x64_context_addresses_at(context, source);

        assert(element_size <= i64_MAX);
        i64 offset = (i64)element_size;
        target_element_address.offset += offset;
        source_element_address.offset += offset;

        target = x64_context_addresses_insert(context, target_element_address);
        source = x64_context_addresses_insert(context, source_element_address);
    }
}

void x64_codegen_copy_memory(u16 target,
                             u16 source,
                             Type const *type,
                             u64 block_index,
                             x64_Context *context) {
    if (type_is_scalar(type)) {
        x64_codegen_copy_scalar_memory(target, source, block_index, context);
    } else {
        x64_codegen_copy_composite_memory(
            target, source, type, block_index, context);
    }
}

void x64_codegen_copy_allocation_from_memory(x64_Allocation *target,
                                             u16 source,
                                             Type const *type,
                                             u64 block_index,
                                             x64_Context *context) {
    if (target->location.kind == LOCATION_ADDRESS) {
        x64_codegen_copy_memory(
            target->location.address, source, type, block_index, context);
    } else {
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(target->location.gpr),
                                   x64_operand_address(source)));
    }
}

static void x64_codegen_copy_scalar_allocation(x64_Allocation *target,
                                               x64_Allocation *source,
                                               u64 block_index,
                                               x64_Context *context) {
    if ((target->location.kind == LOCATION_GPR) ||
        (source->location.kind == LOCATION_GPR)) {
        x64_context_append(context,
                           x64_mov(x64_operand_location(target->location),
                                   x64_operand_location(source->location)));
    } else {
        x64_codegen_copy_scalar_memory(target->location.address,
                                       source->location.address,
                                       block_index,
                                       context);
    }
}

void x64_codegen_copy_allocation(x64_Allocation *target,
                                 x64_Allocation *source,
                                 u64 block_index,
                                 x64_Context *context) {
    assert(type_equality(target->type, source->type));

    if (x64_location_eq(target->location, source->location)) { return; }

    if (type_is_scalar(target->type)) {
        x64_codegen_copy_scalar_allocation(
            target, source, block_index, context);
    } else {
        assert(target->location.kind == LOCATION_ADDRESS);
        assert(source->location.kind == LOCATION_ADDRESS);

        x64_codegen_copy_composite_memory(target->location.address,
                                          source->location.address,
                                          target->type,
                                          block_index,
                                          context);
    }
}

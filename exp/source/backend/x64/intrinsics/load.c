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

static void x64_codegen_load_i32(x64_Address target_address,
                                 i32 value,
                                 x64_Context *x64_context) {
    if (i64_in_range_i32(value)) {
        x64_context_append(x64_context,
                           x64_mov(x64_operand_address(target_address),
                                   x64_operand_immediate((i32)value)));
    } else {
        Operand operand = context_constants_append(x64_context->context,
                                                   value_create_i32(value));
        assert(operand.kind == OPERAND_KIND_CONSTANT);
        x64_context_append(
            x64_context,
            x64_mov(x64_operand_address(target_address),
                    x64_operand_constant(operand.data.constant)));
    }
}

static void x64_codegen_load_address_from_scalar_value(
    x64_Address target_address, Value *value, x64_Context *context) {
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: break; // don't initialize the uninitialized

    case VALUE_KIND_NIL: {
        x64_context_append(context,
                           x64_mov(x64_operand_address(target_address),
                                   x64_operand_immediate(0)));
        break;
    }

    case VALUE_KIND_BOOLEAN: {
        x64_context_append(context,
                           x64_mov(x64_operand_address(target_address),
                                   x64_operand_immediate((i32)value->boolean)));
        break;
    }

    case VALUE_KIND_I32: {
        x64_codegen_load_i32(target_address, value->i32_, context);
        break;
    }

    case VALUE_KIND_TUPLE:
    default:               EXP_UNREACHABLE();
    }
}

static void x64_codegen_load_address_from_scalar_operand(x64_Address target,
                                                         Operand source,
                                                         Type const *type,
                                                         u64 block_index,
                                                         x64_Context *context) {
    assert(type_is_scalar(type));

    switch (source.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, source.data.ssa);
        if (allocation->location.kind == LOCATION_GPR) {
            x64_context_append(
                context,
                x64_mov(x64_operand_address(target),
                        x64_operand_gpr(allocation->location.gpr)));
        } else {
            x64_codegen_copy_scalar_memory(target,
                                           allocation->location.address,
                                           size_of(type),
                                           block_index,
                                           context);
        }
        break;
    }

    case OPERAND_KIND_I32: {
        x64_context_append(context,
                           x64_mov(x64_operand_address(target),
                                   x64_operand_immediate(source.data.i32_)));
        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *value = x64_context_constants_at(context, source.data.constant);
        assert(type_equality(type, type_of_value(value, context->context)));
        x64_codegen_load_address_from_scalar_value(target, value, context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

static void
x64_codegen_load_address_from_composite_operand(x64_Address target,
                                                Operand source,
                                                Type const *type,
                                                u64 block_index,
                                                x64_Context *context) {
    switch (source.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, source.data.ssa);

        assert(allocation->location.kind == LOCATION_ADDRESS);

        x64_codegen_copy_composite_memory(
            target, allocation->location.address, type, block_index, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *value = x64_context_constants_at(context, source.data.constant);
        Type const *type = type_of_value(value, context->context);
        assert(value->kind == VALUE_KIND_TUPLE);
        assert(!type_is_scalar(type));
        (void)type;
        Tuple *tuple = &value->tuple;

        for (u64 i = 0; i < tuple->size; ++i) {
            Operand element = tuple->elements[i];
            Type const *element_type =
                type_of_operand(element, context->context);
            u64 element_size = size_of(element_type);

            x64_codegen_load_address_from_operand(
                target, element, element_type, block_index, context);

            assert(element_size <= i64_MAX);
            i64 offset = (i64)element_size;
            target.offset += offset;
        }

        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    // #NOTE: an immediate value is never composite
    case OPERAND_KIND_I32:
    default:               EXP_UNREACHABLE();
    }
}

void x64_codegen_load_address_from_operand(x64_Address target,
                                           Operand source,
                                           Type const *type,
                                           u64 block_index,
                                           x64_Context *context) {
    if (type_is_scalar(type)) {
        x64_codegen_load_address_from_scalar_operand(
            target, source, type, block_index, context);
    } else {
        x64_codegen_load_address_from_composite_operand(
            target, source, type, block_index, context);
    }
}

static void
x64_codegen_load_argument_from_scalar_operand(x64_Address target,
                                              Operand source,
                                              Type const *type,
                                              u64 block_index,
                                              x64_Context *context) {
    switch (source.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, source.data.ssa);
        if (allocation->location.kind == LOCATION_GPR) {
            x64_context_append(
                context,
                x64_mov(x64_operand_address(target),
                        x64_operand_gpr(allocation->location.gpr)));
        } else {
            x64_codegen_copy_scalar_memory(target,
                                           allocation->location.address,
                                           size_of(type),
                                           block_index,
                                           context);
        }
        break;
    }

    case OPERAND_KIND_I32: {
        x64_context_append(context,
                           x64_mov(x64_operand_address(target),
                                   x64_operand_immediate(source.data.i32_)));
        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *value = x64_context_constants_at(context, source.data.constant);
        x64_codegen_load_address_from_scalar_value(target, value, context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

static void
x64_codegen_load_argument_from_composite_operand(x64_Address target,
                                                 Operand source,
                                                 Type const *type,
                                                 u64 block_index,
                                                 x64_Context *context) {
    switch (source.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, source.data.ssa);

        assert(allocation->location.kind == LOCATION_ADDRESS);

        x64_codegen_copy_composite_memory(
            target, allocation->location.address, type, block_index, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *value = x64_context_constants_at(context, source.data.constant);
        Type const *type = type_of_value(value, context->context);
        assert(value->kind == VALUE_KIND_TUPLE);
        assert(!type_is_scalar(type));
        (void)type;
        Tuple *tuple = &value->tuple;

        for (u64 i = 0; i < tuple->size; ++i) {
            Operand element = tuple->elements[i];
            Type const *element_type =
                type_of_operand(element, context->context);
            u64 element_size = size_of(element_type);

            x64_codegen_load_argument_from_operand(
                target, element, element_type, block_index, context);

            assert(element_size <= i64_MAX);
            i64 offset = -(i64)element_size;
            target.offset += offset;
        }

        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    // #NOTE: immediate operands are never composite
    case OPERAND_KIND_I32:
    default:               EXP_UNREACHABLE();
    }
}

void x64_codegen_load_argument_from_operand(x64_Address target,
                                            Operand source,
                                            Type const *type,
                                            u64 block_index,
                                            x64_Context *context) {
    if (type_is_scalar(type)) {
        x64_codegen_load_argument_from_scalar_operand(
            target, source, type, block_index, context);
    } else {
        x64_codegen_load_argument_from_composite_operand(
            target, source, type, block_index, context);
    }
}

void x64_codegen_load_gpr_from_operand(x64_GPR gpr,
                                       Operand source,
                                       [[maybe_unused]] u64 block_index,
                                       x64_Context *context) {
    switch (source.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, source.data.ssa);
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_location(allocation->location)));
        break;
    }

    case OPERAND_KIND_I32: {
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_immediate(source.data.i32_)));
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        PANIC("#TODO");
        break;
    }

    // we don't create globals that are not functions (yet)
    case OPERAND_KIND_LABEL:
    default:                 EXP_UNREACHABLE();
    }
}

void x64_codegen_load_allocation_from_operand(x64_Allocation *target,
                                              Operand source,
                                              u64 block_index,
                                              x64_Context *context) {
    if (target->location.kind == LOCATION_ADDRESS) {
        x64_codegen_load_address_from_operand(target->location.address,
                                              source,
                                              target->type,
                                              block_index,
                                              context);
    } else {
        x64_codegen_load_gpr_from_operand(
            target->location.gpr, source, block_index, context);
    }
}

static void x64_codegen_load_allocation_from_i32(x64_Allocation *target,
                                                 i32 value,
                                                 x64_Context *context) {
    x64_context_append(context,
                       x64_mov(x64_operand_location(target->location),
                               x64_operand_immediate(value)));
}

static void x64_codegen_load_allocation_from_tuple(x64_Allocation *target,
                                                   Tuple *tuple,
                                                   u64 block_index,
                                                   x64_Context *context) {
    assert(target->location.kind == LOCATION_ADDRESS);
    x64_Address address = target->location.address;
    for (u64 i = 0; i < tuple->size; ++i) {
        Operand element          = tuple->elements[i];
        Type const *element_type = type_of_operand(element, context->context);
        u64 element_size         = size_of(element_type);

        x64_codegen_load_address_from_operand(
            address, element, element_type, block_index, context);

        assert(element_size <= i64_MAX);
        i64 offset = (i64)element_size;
        address.offset += offset;
    }
}

void x64_codegen_load_allocation_from_value(x64_Allocation *target,
                                            Value *value,
                                            u64 block_index,
                                            x64_Context *x64_context) {
    Type const *type = type_of_value(value, x64_context->context);
    assert(type_equality(target->type, type));
    (void)type;

    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: break;

    case VALUE_KIND_NIL: {
        x64_context_append(x64_context,
                           x64_mov(x64_operand_location(target->location),
                                   x64_operand_immediate(0)));
        break;
    }

    case VALUE_KIND_BOOLEAN: {
        x64_context_append(x64_context,
                           x64_mov(x64_operand_location(target->location),
                                   x64_operand_immediate((i16)value->boolean)));
        break;
    }

    case VALUE_KIND_I32: {
        x64_codegen_load_allocation_from_i32(target, value->i32_, x64_context);
        break;
    }

    case VALUE_KIND_TUPLE: {
        x64_codegen_load_allocation_from_tuple(
            target, &value->tuple, block_index, x64_context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

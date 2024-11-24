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

static void
x64_codegen_load_i64(x64_Address *dst, i64 value, x64_Context *x64_context) {
    if (i64_in_range_i16(value)) {
        x64_context_append(x64_context,
                           x64_mov(x64_operand_address(*dst),
                                   x64_operand_immediate((i16)value)));
    } else {
        Operand operand = context_constants_append(x64_context->context,
                                                   value_create_i64(value));
        assert(operand.kind == OPERAND_KIND_CONSTANT);
        x64_context_append(
            x64_context,
            x64_mov(x64_operand_address(*dst),
                    x64_operand_constant(operand.data.constant)));
    }
}

static void x64_codegen_load_address_from_scalar_value(x64_Address *dst,
                                                       Value *value,
                                                       x64_Context *context) {
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: break; // don't initialize the uninitialized

    case VALUE_KIND_NIL: {
        x64_context_append(
            context,
            x64_mov(x64_operand_address(*dst), x64_operand_immediate(0)));
        break;
    }

    case VALUE_KIND_BOOLEAN: {
        x64_context_append(context,
                           x64_mov(x64_operand_address(*dst),
                                   x64_operand_immediate((i16)value->boolean)));
        break;
    }

    case VALUE_KIND_I64: {
        x64_codegen_load_i64(dst, value->i64_, context);
        break;
    }

    case VALUE_KIND_TUPLE:
    default:               EXP_UNREACHABLE();
    }
}

static void
x64_codegen_load_address_from_scalar_operand(x64_Address *dst,
                                             Operand src,
                                             [[maybe_unused]] Type const *type,
                                             u64 Idx,
                                             x64_Context *context) {
    assert(type_is_scalar(type));

    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, src.data.ssa);
        if (allocation->location.kind == LOCATION_GPR) {
            x64_context_append(
                context,
                x64_mov(x64_operand_address(*dst),
                        x64_operand_gpr(allocation->location.gpr)));
        } else {
            x64_codegen_copy_scalar_memory(
                dst, &allocation->location.address, Idx, context);
        }
        break;
    }

    case OPERAND_KIND_IMMEDIATE: {
        x64_context_append(context,
                           x64_mov(x64_operand_address(*dst),
                                   x64_operand_immediate(src.data.immediate)));
        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *value = x64_context_value_at(context, src.data.constant);
        assert(type_equality(type, type_of_value(value, context->context)));
        x64_codegen_load_address_from_scalar_value(dst, value, context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

static void
x64_codegen_load_address_from_composite_operand(x64_Address *dst,
                                                Operand src,
                                                Type const *type,
                                                u64 Idx,
                                                x64_Context *context) {
    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, src.data.ssa);

        assert(allocation->location.kind == LOCATION_ADDRESS);

        x64_codegen_copy_composite_memory(
            dst, &allocation->location.address, type, Idx, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *value     = x64_context_value_at(context, src.data.constant);
        Type const *type = type_of_value(value, context->context);
        assert(value->kind == VALUE_KIND_TUPLE);
        assert(!type_is_scalar(type));
        (void)type;
        Tuple *tuple = &value->tuple;

        x64_Address dst_element_address = *dst;
        for (u64 i = 0; i < tuple->size; ++i) {
            Operand element = tuple->elements[i];
            Type const *element_type =
                type_of_operand(element, context->context);
            u64 element_size = size_of(element_type);

            x64_codegen_load_address_from_operand(
                &dst_element_address, element, element_type, Idx, context);

            assert(element_size <= i64_MAX);
            i64 offset = (i64)element_size;
            dst_element_address.offset += offset;
        }

        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    // #NOTE: an immediate value is never composite
    case OPERAND_KIND_IMMEDIATE:
    default:                     EXP_UNREACHABLE();
    }
}

void x64_codegen_load_address_from_operand(x64_Address *dst,
                                           Operand src,
                                           Type const *type,
                                           u64 Idx,
                                           x64_Context *context) {
    if (type_is_scalar(type)) {
        x64_codegen_load_address_from_scalar_operand(
            dst, src, type, Idx, context);
    } else {
        x64_codegen_load_address_from_composite_operand(
            dst, src, type, Idx, context);
    }
}

static void
x64_codegen_load_argument_from_scalar_operand(x64_Address *dst,
                                              Operand src,
                                              [[maybe_unused]] Type const *type,
                                              u64 Idx,
                                              x64_Context *context) {
    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, src.data.ssa);
        if (allocation->location.kind == LOCATION_GPR) {
            x64_context_append(
                context,
                x64_mov(x64_operand_address(*dst),
                        x64_operand_gpr(allocation->location.gpr)));
        } else {
            x64_codegen_copy_scalar_memory(
                dst, &allocation->location.address, Idx, context);
        }
        break;
    }

    case OPERAND_KIND_IMMEDIATE: {
        x64_context_append(context,
                           x64_mov(x64_operand_address(*dst),
                                   x64_operand_immediate(src.data.immediate)));
        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *value = x64_context_value_at(context, src.data.constant);
        x64_codegen_load_address_from_scalar_value(dst, value, context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

static void
x64_codegen_load_argument_from_composite_operand(x64_Address *dst,
                                                 Operand src,
                                                 Type const *type,
                                                 u64 Idx,
                                                 x64_Context *context) {
    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, src.data.ssa);

        assert(allocation->location.kind == LOCATION_ADDRESS);

        x64_codegen_copy_composite_memory(
            dst, &allocation->location.address, type, Idx, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *value     = x64_context_value_at(context, src.data.constant);
        Type const *type = type_of_value(value, context->context);
        assert(value->kind == VALUE_KIND_TUPLE);
        assert(!type_is_scalar(type));
        (void)type;
        Tuple *tuple = &value->tuple;

        x64_Address dst_element_address = *dst;
        for (u64 i = 0; i < tuple->size; ++i) {
            Operand element = tuple->elements[i];
            Type const *element_type =
                type_of_operand(element, context->context);
            u64 element_size = size_of(element_type);

            x64_codegen_load_argument_from_operand(
                &dst_element_address, element, element_type, Idx, context);

            assert(element_size <= i64_MAX);
            i64 offset = -(i64)element_size;
            dst_element_address.offset += offset;
        }

        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    // #NOTE: immediate operands are never composite
    case OPERAND_KIND_IMMEDIATE:
    default:                     EXP_UNREACHABLE();
    }
}

void x64_codegen_load_argument_from_operand(x64_Address *dst,
                                            Operand src,
                                            Type const *type,
                                            u64 Idx,
                                            x64_Context *context) {
    if (type_is_scalar(type)) {
        x64_codegen_load_argument_from_scalar_operand(
            dst, src, type, Idx, context);
    } else {
        x64_codegen_load_argument_from_composite_operand(
            dst, src, type, Idx, context);
    }
}

void x64_codegen_load_gpr_from_operand(x64_GPR gpr,
                                       Operand src,
                                       [[maybe_unused]] u64 Idx,
                                       x64_Context *context) {
    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x64_Allocation *allocation =
            x64_context_allocation_of(context, src.data.ssa);
        x64_context_append(
            context,
            x64_mov(x64_operand_gpr(gpr), x64_operand_alloc(allocation)));
        break;
    }

    case OPERAND_KIND_IMMEDIATE: {
        x64_context_append(context,
                           x64_mov(x64_operand_gpr(gpr),
                                   x64_operand_immediate(src.data.immediate)));
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

void x64_codegen_load_allocation_from_operand(x64_Allocation *dst,
                                              Operand src,
                                              u64 Idx,
                                              x64_Context *context) {
    if (dst->location.kind == LOCATION_ADDRESS) {
        x64_codegen_load_address_from_operand(
            &dst->location.address, src, dst->type, Idx, context);
    } else {
        x64_codegen_load_gpr_from_operand(dst->location.gpr, src, Idx, context);
    }
}

static void x64_codegen_load_allocation_from_i64(x64_Allocation *dst,
                                                 i64 value,
                                                 x64_Context *context) {
    if (i64_in_range_i16(value)) {
        x64_context_append(
            context,
            x64_mov(x64_operand_alloc(dst), x64_operand_immediate((i16)value)));
    } else {
        Operand operand =
            context_constants_append(context->context, value_create_i64(value));
        assert(operand.kind == OPERAND_KIND_CONSTANT);
        x64_context_append(
            context,
            x64_mov(x64_operand_alloc(dst),
                    x64_operand_constant(operand.data.constant)));
    }
}

static void x64_codegen_load_allocation_from_tuple(x64_Allocation *dst,
                                                   Tuple *tuple,
                                                   u64 Idx,
                                                   x64_Context *context) {
    assert(dst->location.kind == LOCATION_ADDRESS);
    x64_Address dst_address = dst->location.address;
    for (u64 i = 0; i < tuple->size; ++i) {
        Operand element          = tuple->elements[i];
        Type const *element_type = type_of_operand(element, context->context);
        u64 element_size         = size_of(element_type);

        x64_codegen_load_address_from_operand(
            &dst_address, element, element_type, Idx, context);

        assert(element_size <= i64_MAX);
        i64 offset = (i64)element_size;
        dst_address.offset += offset;
    }
}

void x64_codegen_load_allocation_from_value(x64_Allocation *dst,
                                            Value *value,
                                            u64 Idx,
                                            x64_Context *x64_context) {
    Type const *type = type_of_value(value, x64_context->context);
    assert(type_equality(dst->type, type));
    (void)type;

    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: break;

    case VALUE_KIND_NIL: {
        x64_context_append(
            x64_context,
            x64_mov(x64_operand_alloc(dst), x64_operand_immediate(0)));
        break;
    }

    case VALUE_KIND_BOOLEAN: {
        x64_context_append(x64_context,
                           x64_mov(x64_operand_alloc(dst),
                                   x64_operand_immediate((i16)value->boolean)));
        break;
    }

    case VALUE_KIND_I64: {
        x64_codegen_load_allocation_from_i64(dst, value->i64_, x64_context);
        break;
    }

    case VALUE_KIND_TUPLE: {
        x64_codegen_load_allocation_from_tuple(
            dst, &value->tuple, Idx, x64_context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

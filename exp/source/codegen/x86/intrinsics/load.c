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

#include "codegen/x86/intrinsics/copy.h"
#include "codegen/x86/intrinsics/load.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "support/assert.h"
#include "support/panic.h"
#include "support/unreachable.h"

static void
x86_codegen_load_i64(x86_Address *dst, i64 value, x86_Context *x64_context) {
    x86_context_append(
        x64_context,
        x86_mov(x86_operand_address(*dst), x86_operand_i64(value)));
}

static void
x86_codegen_load_address_from_scalar_value(x86_Address *restrict dst,
                                           Value const *restrict value,
                                           x86_Context *restrict context) {
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: break; // don't initialize the uninitialized

    case VALUE_KIND_NIL: {
        x86_context_append(
            context, x86_mov(x86_operand_address(*dst), x86_operand_i64(0)));
        break;
    }

    case VALUE_KIND_BOOLEAN: {
        x86_context_append(context,
                           x86_mov(x86_operand_address(*dst),
                                   x86_operand_i64(value->boolean)));
        break;
    }

    case VALUE_KIND_I64: {
        x86_codegen_load_i64(dst, value->i64_, context);
        break;
    }

    case VALUE_KIND_TUPLE:
    default:               EXP_UNREACHABLE();
    }
}

static void
x86_codegen_load_address_from_scalar_operand(x86_Address *restrict dst,
                                             Operand src,
                                             Type const *restrict type,
                                             u64 Idx,
                                             x86_Context *restrict context) {
    assert(type_is_scalar(type));

    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x86_Allocation *allocation =
            x86_context_allocation_of(context, src.data.ssa);
        if (allocation->location.kind == X86_LOCATION_GPR) {
            x86_context_append(
                context,
                x86_mov(x86_operand_address(*dst),
                        x86_operand_gpr(allocation->location.gpr)));
        } else {
            u64 size = x86_size_of(type);
            x86_codegen_copy_scalar_memory(
                dst, &allocation->location.address, size, Idx, context);
        }
        break;
    }

    case OPERAND_KIND_I64: {
        x86_context_append(
            context,
            x86_mov(x86_operand_address(*dst), x86_operand_i64(src.data.i64_)));
        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *value = src.data.constant;
        assert(type_equality(type, type_of_value(value, context->context)));
        x86_codegen_load_address_from_scalar_value(dst, value, context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

static void
x86_codegen_load_address_from_composite_operand(x86_Address *restrict dst,
                                                Operand src,
                                                Type const *restrict type,
                                                u64 Idx,
                                                x86_Context *restrict context) {
    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x86_Allocation *allocation =
            x86_context_allocation_of(context, src.data.ssa);

        assert(allocation->location.kind == X86_LOCATION_ADDRESS);

        x86_codegen_copy_composite_memory(
            dst, &allocation->location.address, type, Idx, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *value = src.data.constant;
        Type const  *type  = type_of_value(value, context->context);
        assert(value->kind == VALUE_KIND_TUPLE);
        assert(!type_is_scalar(type));
        (void)type;
        Tuple const *tuple = &value->tuple;

        x86_Address dst_element_address = *dst;
        for (u64 i = 0; i < tuple->size; ++i) {
            Operand     element = tuple->elements[i];
            Type const *element_type =
                type_of_operand(element, context->context);
            u64 element_size = x86_size_of(element_type);

            x86_codegen_load_address_from_operand(
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
    case OPERAND_KIND_I64:
    default:               EXP_UNREACHABLE();
    }
}

void x86_codegen_load_address_from_operand(x86_Address *restrict dst,
                                           Operand src,
                                           Type const *restrict type,
                                           u64 Idx,
                                           x86_Context *restrict context) {
    if (type_is_scalar(type)) {
        x86_codegen_load_address_from_scalar_operand(
            dst, src, type, Idx, context);
    } else {
        x86_codegen_load_address_from_composite_operand(
            dst, src, type, Idx, context);
    }
}

static void
x86_codegen_load_argument_from_scalar_operand(x86_Address *restrict dst,
                                              Operand src,
                                              Type const *restrict type,
                                              u64 Idx,
                                              x86_Context *restrict context) {
    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x86_Allocation *allocation =
            x86_context_allocation_of(context, src.data.ssa);
        if (allocation->location.kind == X86_LOCATION_GPR) {
            x86_context_append(
                context,
                x86_mov(x86_operand_address(*dst),
                        x86_operand_gpr(allocation->location.gpr)));
        } else {
            u64 size = x86_size_of(type);
            x86_codegen_copy_scalar_memory(
                dst, &allocation->location.address, size, Idx, context);
        }
        break;
    }

    case OPERAND_KIND_I64: {
        x86_context_append(
            context,
            x86_mov(x86_operand_address(*dst), x86_operand_i64(src.data.i64_)));
        break;
    }

    case OPERAND_KIND_LABEL: {
        PANIC("#TODO #GLOBAL_CONSTANTS");
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *value = src.data.constant;
        x86_codegen_load_address_from_scalar_value(dst, value, context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

static void x86_codegen_load_argument_from_composite_operand(
    x86_Address *restrict dst,
    Operand src,
    Type const *restrict type,
    u64 Idx,
    x86_Context *restrict context) {
    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x86_Allocation *allocation =
            x86_context_allocation_of(context, src.data.ssa);

        assert(allocation->location.kind == X86_LOCATION_ADDRESS);

        x86_codegen_copy_composite_memory(
            dst, &allocation->location.address, type, Idx, context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *value = src.data.constant;
        Type const  *type  = type_of_value(value, context->context);
        assert(value->kind == VALUE_KIND_TUPLE);
        assert(!type_is_scalar(type));
        (void)type;
        Tuple const *tuple = &value->tuple;

        x86_Address dst_element_address = *dst;
        for (u64 i = 0; i < tuple->size; ++i) {
            Operand     element = tuple->elements[i];
            Type const *element_type =
                type_of_operand(element, context->context);
            u64 element_size = x86_size_of(element_type);

            x86_codegen_load_argument_from_operand(
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
    case OPERAND_KIND_I64:
    default:               EXP_UNREACHABLE();
    }
}

void x86_codegen_load_argument_from_operand(x86_Address *restrict dst,
                                            Operand src,
                                            Type const *restrict type,
                                            u64 Idx,
                                            x86_Context *restrict context) {
    if (type_is_scalar(type)) {
        x86_codegen_load_argument_from_scalar_operand(
            dst, src, type, Idx, context);
    } else {
        x86_codegen_load_argument_from_composite_operand(
            dst, src, type, Idx, context);
    }
}

void x86_codegen_load_gpr_from_operand(x86_GPR              gpr,
                                       Operand              src,
                                       [[maybe_unused]] u64 Idx,
                                       x86_Context *restrict context) {
    switch (src.kind) {
    case OPERAND_KIND_SSA: {
        x86_Allocation *allocation =
            x86_context_allocation_of(context, src.data.ssa);
        u64 size = x86_size_of(allocation->type);
        exp_assert_debug(x86_gpr_valid_size(size));
        x86_GPR sized_gpr = x86_gpr_resize(gpr, size);
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(sized_gpr), x86_operand_alloc(allocation)));
        break;
    }

    case OPERAND_KIND_I64: {
        x86_context_append(
            context,
            x86_mov(x86_operand_gpr(gpr), x86_operand_i64(src.data.i64_)));
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

void x86_codegen_load_allocation_from_operand(x86_Allocation *restrict dst,
                                              Operand src,
                                              u64     Idx,
                                              x86_Context *restrict context) {
    if (dst->location.kind == X86_LOCATION_ADDRESS) {
        x86_codegen_load_address_from_operand(
            &dst->location.address, src, dst->type, Idx, context);
    } else {
        x86_codegen_load_gpr_from_operand(dst->location.gpr, src, Idx, context);
    }
}

static void x86_codegen_load_allocation_from_i64(x86_Allocation *dst,
                                                 i64             value,
                                                 x86_Context    *context) {
    x86_context_append(context,
                       x86_mov(x86_operand_alloc(dst), x86_operand_i64(value)));
}

static void x86_codegen_load_allocation_from_tuple(x86_Allocation *dst,
                                                   Tuple const    *tuple,
                                                   u64             Idx,
                                                   x86_Context    *context) {
    assert(dst->location.kind == X86_LOCATION_ADDRESS);
    x86_Address dst_address = dst->location.address;
    for (u64 i = 0; i < tuple->size; ++i) {
        Operand     element      = tuple->elements[i];
        Type const *element_type = type_of_operand(element, context->context);
        u64         element_size = x86_size_of(element_type);

        x86_codegen_load_address_from_operand(
            &dst_address, element, element_type, Idx, context);

        assert(element_size <= i64_MAX);
        i64 offset = (i64)element_size;
        dst_address.offset += offset;
    }
}

void x86_codegen_load_allocation_from_constant(
    x86_Allocation *restrict dst,
    Value const *value,
    u64          Idx,
    x86_Context *restrict x64_context) {
    Type const *type = type_of_value(value, x64_context->context);
    assert(type_equality(dst->type, type));
    (void)type;

    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: break;

    case VALUE_KIND_NIL: {
        x86_context_append(x64_context,
                           x86_mov(x86_operand_alloc(dst), x86_operand_i64(0)));
        break;
    }

    case VALUE_KIND_BOOLEAN: {
        x86_context_append(x64_context,
                           x86_mov(x86_operand_alloc(dst),
                                   x86_operand_i64((i16)value->boolean)));
        break;
    }

    case VALUE_KIND_I64: {
        x86_codegen_load_allocation_from_i64(dst, value->i64_, x64_context);
        break;
    }

    case VALUE_KIND_TUPLE: {
        x86_codegen_load_allocation_from_tuple(
            dst, &value->tuple, Idx, x64_context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

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

#include "codegen/x86/instruction/call.h"
#include "codegen/x86/intrinsics/load.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/message.h"

typedef struct OperandArray {
    u8       size;
    u8       capacity;
    Operand *buffer;
} OperandArray;

static OperandArray operand_array_create() {
    OperandArray array = {.size = 0, .capacity = 0, .buffer = nullptr};
    return array;
}

static void operand_array_destroy(OperandArray *restrict array) {
    deallocate(array->buffer);
    array->size     = 0;
    array->capacity = 0;
    array->buffer   = nullptr;
}

static bool operand_array_full(OperandArray *restrict array) {
    return (array->size + 1) >= array->capacity;
}

static void operand_array_grow(OperandArray *restrict array) {
    Growth_u8 g     = array_growth_u8(array->capacity, sizeof(Operand));
    array->buffer   = reallocate(array->buffer, g.alloc_size);
    array->capacity = g.new_capacity;
}

static void operand_array_append(OperandArray *restrict array,
                                 Operand operand) {
    if (operand_array_full(array)) { operand_array_grow(array); }
    array->buffer[array->size++] = operand;
}

static void x86_codegen_allocate_stack_space_for_arguments(x86_Context *context,
                                                           i64 stack_space,
                                                           u64 block_index) {
    if (i64_in_range_i16(stack_space)) {
        x86_context_insert(context,
                           x86_sub(x86_operand_gpr(X86_GPR_RSP),
                                   x86_operand_immediate((i16)stack_space)),
                           block_index);
    } else {
        Operand operand = context_constants_append(
            context->context, value_create_i64(stack_space));
        assert(operand.kind == OPERAND_KIND_CONSTANT);
        x86_context_insert(context,
                           x86_sub(x86_operand_gpr(X86_GPR_RSP),
                                   x86_operand_constant(operand.data.constant)),
                           block_index);
    }
}

static void
x86_codegen_deallocate_stack_space_for_arguments(x86_Context *x64_context,
                                                 i64          stack_space) {
    if (i64_in_range_i16(stack_space)) {
        x86_context_append(x64_context,
                           x86_add(x86_operand_gpr(X86_GPR_RSP),
                                   x86_operand_immediate((i16)stack_space)));
    } else {
        Operand operand = context_constants_append(
            x64_context->context, value_create_i64(stack_space));
        assert(operand.kind == OPERAND_KIND_CONSTANT);
        x86_context_append(
            x64_context,
            x86_add(x86_operand_gpr(X86_GPR_RSP),
                    x86_operand_constant(operand.data.constant)));
    }
}

void x86_codegen_call(Instruction I,
                      u64         block_index,
                      x86_Context *restrict context) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local *local = x86_context_lookup_ssa(context, I.A_data.ssa);
    u8     scalar_argument_count = 0;

    // #NOTE the result of a call expression is either stored in a register
    // (rAX) or on the stack. Iff it is on the stack, it is callee allocated,
    // and the first argument to the function is used to store a pointer to said
    // stack allocation
    if (type_is_scalar(local->type)) {
        x86_context_allocate_to_gpr(context, local, X86_GPR_rAX, block_index);
    } else {
        x86_Allocation *result =
            x86_context_allocate(context, local, block_index);
        assert(result->location.kind == X86_LOCATION_ADDRESS);
        x86_context_append(
            context,
            x86_lea(x86_operand_gpr(
                        x86_gpr_scalar_argument(scalar_argument_count++, 8)),
                    x86_operand_address(result->location.address)));
    }

    Value *value = x86_context_value_at(context, I.C_data.constant);
    assert(value->kind == VALUE_KIND_TUPLE);
    Tuple       *args       = &value->tuple;
    u64          call_start = x86_context_current_offset(context);
    OperandArray stack_args = operand_array_create();

    for (u8 i = 0; i < args->size; ++i) {
        Operand     arg      = args->elements[i];
        Type const *arg_type = type_of_operand(arg, context->context);

        if (type_is_scalar(arg_type) && (scalar_argument_count < 6)) {
            u64 size = size_of(arg_type);
            assert(x86_gpr_valid_size(size));
            x86_GPR gpr =
                x86_gpr_scalar_argument(scalar_argument_count++, size);
            x86_codegen_load_gpr_from_operand(gpr, arg, block_index, context);
        } else {
            operand_array_append(&stack_args, arg);
        }
    }

    if (stack_args.size == 0) {
        x86_context_append(context,
                           x86_call(x86_operand_label(I.B_data.label)));
        return;
    }

    i64         stack_space = 0;
    x86_Address arg_address = x86_address_create(X86_GPR_RSP, 0);

    for (u8 i = 0; i < stack_args.size; ++i) {
        Operand     arg      = stack_args.buffer[i];
        Type const *arg_type = type_of_operand(arg, context->context);
        u64         arg_size = size_of(arg_type);
        assert(arg_size <= i64_MAX);
        i64 offset = (i64)(arg_size);
        stack_space += offset;

        x86_codegen_load_address_from_operand(
            &arg_address, arg, arg_type, block_index, context);

        arg_address.offset += offset;
    }

    x86_codegen_allocate_stack_space_for_arguments(
        context, stack_space, call_start);

    x86_context_append(context, x86_call(x86_operand_label(I.B_data.label)));

    x86_codegen_deallocate_stack_space_for_arguments(context, stack_space);

    operand_array_destroy(&stack_args);
}

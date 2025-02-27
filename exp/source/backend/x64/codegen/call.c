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

#include "backend/x64/codegen/call.h"
#include "backend/x64/intrinsics/load.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/panic.h"

typedef struct OperandArray {
    u8 size;
    u8 capacity;
    Operand *buffer;
} OperandArray;

static OperandArray operand_array_create() {
    OperandArray array = {.size = 0, .capacity = 0, .buffer = nullptr};
    return array;
}

static void operand_array_destroy(OperandArray *array) {
    assert(array != nullptr);
    deallocate(array->buffer);
    array->size     = 0;
    array->capacity = 0;
    array->buffer   = nullptr;
}

static bool operand_array_full(OperandArray *array) {
    assert(array != nullptr);
    return (array->size + 1) >= array->capacity;
}

static void operand_array_grow(OperandArray *array) {
    assert(array != nullptr);
    Growth8 g       = array_growth_u8(array->capacity, sizeof(Operand));
    array->buffer   = reallocate(array->buffer, g.alloc_size);
    array->capacity = g.new_capacity;
}

static void operand_array_append(OperandArray *array, Operand operand) {
    assert(array != nullptr);
    if (operand_array_full(array)) { operand_array_grow(array); }
    array->buffer[array->size++] = operand;
}

static void x64_codegen_allocate_stack_space_for_arguments(
    x64_Context *x64_context, i32 stack_space, u64 block_index) {
    assert(x64_context != nullptr);
    x64_context_insert(x64_context,
                       x64_sub(x64_operand_gpr(X64_GPR_rSP),
                               x64_operand_immediate(stack_space)),
                       block_index);
}

static void
x64_codegen_deallocate_stack_space_for_arguments(x64_Context *x64_context,
                                                 i32 stack_space) {
    assert(x64_context != nullptr);
    x64_context_append(x64_context,
                       x64_add(x64_operand_gpr(X64_GPR_rSP),
                               x64_operand_immediate(stack_space)));
}

void x64_codegen_call(Instruction I, u64 block_index, x64_Context *context) {
    assert(context != nullptr);
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local     = x64_context_lookup_ssa(context, I.A_data.ssa);
    u8 scalar_argument_count = 0;

    if (type_is_scalar(local->type)) {
        x64_context_allocate_to_gpr(context, local, X64_GPR_rAX, block_index);
    } else {
        x64_Allocation *result =
            x64_context_allocate(context, local, block_index);
        assert(result->location.kind == LOCATION_ADDRESS);
        x64_context_append(
            context,
            x64_lea(x64_operand_gpr(x64_gpr_scalar_argument(
                        scalar_argument_count++, size_of(result->type))),
                    x64_operand_address(result->location.address)));
    }

    Value *value = x64_context_constants_at(context, I.C_data.constant);
    assert(value->kind == VALUE_KIND_TUPLE);
    Tuple *args             = &value->tuple;
    u64 call_start          = x64_context_current_offset(context);
    OperandArray stack_args = operand_array_create();

    for (u8 i = 0; i < args->size; ++i) {
        Operand arg          = args->elements[i];
        Type const *arg_type = type_of_operand(arg, context->context);

        if (type_is_scalar(arg_type) && (scalar_argument_count < 6)) {
            x64_GPR gpr = x64_gpr_scalar_argument(scalar_argument_count++,
                                                  size_of(arg_type));
            x64_codegen_load_gpr_from_operand(gpr, arg, block_index, context);
        } else {
            operand_array_append(&stack_args, arg);
        }
    }

    if (stack_args.size == 0) {
        x64_context_append(context,
                           x64_call(x64_operand_label(I.B_data.label)));
        return;
    }

    i32 stack_space         = 0;
    x64_Address arg_address = x64_address_create(X64_GPR_RSP, 0);

    for (u8 i = 0; i < stack_args.size; ++i) {
        Operand arg          = stack_args.buffer[i];
        Type const *arg_type = type_of_operand(arg, context->context);
        u64 arg_size         = size_of(arg_type);
        assert(arg_size <= i32_MAX);
        i32 offset = (i32)(arg_size);
        stack_space += offset;

        x64_codegen_load_address_from_operand(
            arg_address, arg, arg_type, block_index, context);

        arg_address.offset += offset;
    }

    x64_codegen_allocate_stack_space_for_arguments(
        context, stack_space, call_start);

    x64_context_append(context, x64_call(x64_operand_label(I.B_data.label)));

    x64_codegen_deallocate_stack_space_for_arguments(context, stack_space);

    operand_array_destroy(&stack_args);
}

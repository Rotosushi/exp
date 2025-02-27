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

#include "backend/x64/context.h"
#include "env/context.h"

x64_Context x64_context_create(Context *context) {
    assert(context != nullptr);
    x64_Context x64_context = {
        .symbols = x64_symbol_table_create(context->symbol_table.count),
        .context = context,
        .current_function_body     = nullptr,
        .current_x64_function_body = nullptr};
    x64_addresses_initialize(&x64_context.addresses);
    return x64_context;
}

void x64_context_destroy(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    x64_addresses_terminate(&x64_context->addresses);
    x64_symbol_table_destroy(&x64_context->symbols);
}

x64_Symbol *x64_context_symbol(x64_Context *x64_context, StringView name) {
    assert(x64_context != nullptr);
    return x64_symbol_table_at(&x64_context->symbols, name);
}

u32 x64_context_addresses_insert(x64_Context *x64_context,
                                 x64_Address address) {
    assert(x64_context != nullptr);
    u32 index = x64_addresses_insert_unique(&x64_context->addresses, address);
    return index;
}

x64_Address *x64_context_addresses_at(x64_Context *x64_context, u32 index) {
    assert(x64_context != nullptr);
    return x64_addresses_at(&x64_context->addresses, index);
}

Value *x64_context_constants_at(x64_Context *context, u32 index) {
    assert(context != nullptr);
    return context_constants_at(context->context, index);
}

StringView x64_context_labels_at(x64_Context *x64_context, u32 idx) {
    assert(x64_context != nullptr);
    return context_labels_at(x64_context->context, idx);
}

void x64_context_enter_function(x64_Context *x64_context,
                                FunctionBody *body,
                                x64_FunctionBody *x64_body) {
    assert(x64_context != nullptr);
    assert(body != nullptr);
    assert(x64_body != nullptr);
    context_enter_function(x64_context->context, body);
    x64_context->current_function_body     = body;
    x64_context->current_x64_function_body = x64_body;
}

void x64_context_leave_function(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    context_leave_function(x64_context->context);
    x64_context->current_function_body     = nullptr;
    x64_context->current_x64_function_body = nullptr;
}

/*
ActualArgumentList *x64_context_call_at(x64_Context * x64_context,
                                        u64 idx) {
  assert(x64_context != nullptr);
  return context_call_at(x64_context->context, idx);
}
*/

FormalArgument *x64_context_argument_at(x64_Context *x64_context, u8 index) {
    assert(x64_context != nullptr);
    return context_argument_at(x64_context->context, index);
}

FunctionBody *x64_context_current_body(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    assert(x64_context->current_function_body != nullptr);
    return x64_context->current_function_body;
}

Block *x64_context_current_block(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    return &x64_context_current_body(x64_context)->block;
}

LocalVariables *x64_context_current_locals(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    return &x64_context_current_body(x64_context)->locals;
}

x64_FunctionBody *x64_context_current_x64_body(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    assert(x64_context->current_x64_function_body != nullptr);
    return x64_context->current_x64_function_body;
}

x64_Block *x64_context_current_x64_block(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    assert(x64_context->current_x64_function_body != nullptr);
    return &x64_context->current_x64_function_body->block;
}

x64_LocalRegisterAllocator *current_allocator(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    return &x64_context_current_x64_body(x64_context)->allocator;
}

u64 x64_context_current_offset(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    return x64_block_current_offset(x64_context_current_x64_block(x64_context));
}

void x64_context_insert(x64_Context *x64_context,
                        x64_Instruction I,
                        u64 offset) {
    assert(x64_context != nullptr);
    x64_block_insert(x64_context_current_x64_block(x64_context), I, offset);
}

void x64_context_prepend(x64_Context *x64_context, x64_Instruction I) {
    assert(x64_context != nullptr);
    x64_block_prepend(x64_context_current_x64_block(x64_context), I);
}
void x64_context_append(x64_Context *x64_context, x64_Instruction I) {
    assert(x64_context != nullptr);
    x64_block_append(x64_context_current_x64_block(x64_context), I);
}

LocalVariable *x64_context_lookup_ssa(x64_Context *x64_context, u32 ssa) {
    assert(x64_context != nullptr);
    return function_body_locals_ssa(x64_context_current_body(x64_context), ssa);
}

bool x64_context_uses_stack(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    return x64_allocator_uses_stack(current_allocator(x64_context));
}

i32 x64_context_stack_size(x64_Context *x64_context) {
    assert(x64_context != nullptr);
    return x64_allocator_total_stack_size(current_allocator(x64_context));
}

x64_Allocation *x64_context_allocation_of(x64_Context *x64_context, u32 ssa) {
    assert(x64_context != nullptr);
    return x64_allocator_allocation_at(current_allocator(x64_context), ssa);
}

void x64_context_release_gpr(x64_Context *x64_context,
                             x64_GPR gpr,
                             u64 block_index) {
    assert(x64_context != nullptr);
    x64_allocator_release_gpr(current_allocator(x64_context), gpr, block_index);
}

void x64_context_aquire_gpr(x64_Context *x64_context,
                            x64_GPR gpr,
                            u64 block_index) {
    assert(x64_context != nullptr);
    x64_allocator_aquire_gpr(current_allocator(x64_context), gpr, block_index);
}

x64_Allocation *x64_context_allocate(x64_Context *x64_context,
                                     LocalVariable *local,
                                     u64 block_index) {
    assert(x64_context != nullptr);
    assert(local != nullptr);
    return x64_allocator_allocate(
        current_allocator(x64_context), local, block_index);
}

x64_Allocation *x64_context_allocate_from_active(x64_Context *x64_context,
                                                 LocalVariable *local,
                                                 x64_Allocation *active,
                                                 u64 block_index) {
    assert(x64_context != nullptr);
    assert(local != nullptr);
    assert(active != nullptr);
    return x64_allocator_allocate_from_active(
        current_allocator(x64_context), block_index, local, active);
}

x64_Allocation *x64_context_allocate_to_gpr(x64_Context *x64_context,
                                            LocalVariable *local,
                                            x64_GPR gpr,
                                            u64 block_index) {
    assert(x64_context != nullptr);
    assert(local != nullptr);
    return x64_allocator_allocate_to_gpr(
        current_allocator(x64_context), local, gpr, block_index);
}

x64_Allocation *x64_context_allocate_to_address(x64_Context *x64_context,
                                                LocalVariable *local,
                                                x64_Address address) {
    assert(x64_context != nullptr);
    assert(local != nullptr);
    return x64_allocator_allocate_to_address(
        current_allocator(x64_context), local, address);
}

x64_Allocation *x64_context_allocate_result(x64_Context *x64_context,
                                            x64_Location location,
                                            Type *type) {
    assert(x64_context != nullptr);
    return x64_allocator_allocate_result(
        current_allocator(x64_context), location, type);
}

void x64_context_reallocate_active(x64_Context *x64_context,
                                   x64_Allocation *active,
                                   u64 block_index) {
    assert(x64_context != nullptr);
    assert(active != nullptr);
    x64_allocator_reallocate_active(
        current_allocator(x64_context), active, block_index);
}

x64_GPR x64_context_aquire_any_gpr(x64_Context *x64_context,
                                   u64 block_index,
                                   u64 size) {
    assert(x64_context != nullptr);
    return x64_allocator_aquire_any_gpr(
        current_allocator(x64_context), block_index, size);
}

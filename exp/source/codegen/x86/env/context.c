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

#include "codegen/x86/env/context.h"
#include "env/context.h"

x86_Context x86_context_create(Context *context) {
    assert(context != nullptr);
    x86_Context x64_context = {
        .symbols  = x86_symbol_table_create(context->global_symbol_table.count),
        .context  = context,
        .body     = nullptr,
        .x86_body = nullptr};
    return x64_context;
}

void x86_context_destroy(x86_Context *x64_context) {
    assert(x64_context != nullptr);
    x86_symbol_table_destroy(&x64_context->symbols);
}

x86_Symbol *x86_context_symbol(x86_Context *x64_context, StringView name) {
    assert(x64_context != nullptr);
    return x86_symbol_table_at(&x64_context->symbols, name);
}

Value *x86_context_value_at(x86_Context *context, u32 index) {
    assert(context != nullptr);
    return context_constants_at(context->context, index);
}

// StringView x86_context_global_labels_at(x86_Context *x64_context, u32 idx) {
//     assert(x64_context != nullptr);
//     return context_labels_at(x64_context->context, idx);
// }

void x86_context_enter_function(x86_Context *x64_context, StringView name) {
    assert(x64_context != nullptr);
    x64_context->body     = context_enter_function(x64_context->context, name);
    x86_Symbol *symbol    = x86_symbol_table_at(&x64_context->symbols, name);
    x64_context->x86_body = &symbol->body;
    x86_function_create(x64_context->x86_body, x64_context->body);
}

void x86_context_leave_function(x86_Context *x64_context) {
    assert(x64_context != nullptr);
    context_leave_function(x64_context->context);
    x64_context->body     = nullptr;
    x64_context->x86_body = nullptr;
}

Local *x86_context_argument_at(x86_Context *x64_context, u8 index) {
    assert(x64_context != nullptr);
    return context_lookup_argument(x64_context->context, index);
}

Function *x86_context_current_body(x86_Context *x64_context) {
    assert(x64_context->body != nullptr);
    return x64_context->body;
}

Bytecode *x86_context_current_bc(x86_Context *x64_context) {
    return &x86_context_current_body(x64_context)->bc;
}

Locals *x86_context_current_locals(x86_Context *x64_context) {
    return &x86_context_current_body(x64_context)->locals;
}

x86_Function *x86_context_current_x86_body(x86_Context *x64_context) {
    assert(x64_context->x86_body != nullptr);
    return x64_context->x86_body;
}

x86_Bytecode *x86_context_current_x86_bc(x86_Context *x64_context) {
    assert(x64_context->x86_body != nullptr);
    return &x64_context->x86_body->bc;
}

x86_Allocator *current_allocator(x86_Context *x64_context) {
    return &x86_context_current_x86_body(x64_context)->allocator;
}

u64 x86_context_current_offset(x86_Context *x64_context) {
    return x86_bytecode_current_offset(x86_context_current_x86_bc(x64_context));
}

void x86_context_insert(x86_Context    *x64_context,
                        x86_Instruction I,
                        u64             offset) {
    x86_bytecode_insert(x86_context_current_x86_bc(x64_context), I, offset);
}

void x86_context_prepend(x86_Context *x64_context, x86_Instruction I) {
    x86_bytecode_prepend(x86_context_current_x86_bc(x64_context), I);
}
void x86_context_append(x86_Context *x64_context, x86_Instruction I) {
    x86_bytecode_append(x86_context_current_x86_bc(x64_context), I);
}

Local *x86_context_lookup_ssa(x86_Context *x64_context, u32 ssa) {
    assert(x64_context != nullptr);
    return function_lookup_local(x86_context_current_body(x64_context), ssa);
}

bool x86_context_uses_stack(x86_Context *x64_context) {
    assert(x64_context != nullptr);
    return x86_allocator_uses_stack(current_allocator(x64_context));
}

i64 x86_context_stack_size(x86_Context *x64_context) {
    assert(x64_context != nullptr);
    return x86_allocator_total_stack_size(current_allocator(x64_context));
}

x86_Allocation *x86_context_allocation_of(x86_Context *x64_context, u32 ssa) {
    assert(x64_context != nullptr);
    return x86_allocator_allocation_of(current_allocator(x64_context), ssa);
}

void x86_context_release_gpr(x86_Context *x64_context, x86_GPR gpr, u64 Idx) {
    assert(x64_context != nullptr);
    x86_allocator_release_gpr(current_allocator(x64_context),
                              gpr,
                              Idx,
                              x86_context_current_x86_bc(x64_context));
}

void x86_context_aquire_gpr(x86_Context *x64_context, x86_GPR gpr, u64 Idx) {
    assert(x64_context != nullptr);
    x86_allocator_aquire_gpr(current_allocator(x64_context),
                             gpr,
                             Idx,
                             x86_context_current_x86_bc(x64_context));
}

x86_Allocation *
x86_context_allocate(x86_Context *x64_context, Local *local, u64 Idx) {
    return x86_allocator_allocate(current_allocator(x64_context),
                                  Idx,
                                  local,
                                  x86_context_current_x86_bc(x64_context));
}

x86_Allocation *x86_context_allocate_from_active(x86_Context    *x64_context,
                                                 Local          *local,
                                                 x86_Allocation *active,
                                                 u64             Idx) {
    return x86_allocator_allocate_from_active(
        current_allocator(x64_context),
        Idx,
        local,
        active,
        x86_context_current_x86_bc(x64_context));
}

x86_Allocation *x86_context_allocate_to_any_gpr(x86_Context *x64_context,
                                                Local       *local) {
    assert(x64_context != nullptr);
    return x86_allocator_allocate_to_any_gpr(
        current_allocator(x64_context),
        local,
        x86_context_current_x86_bc(x64_context));
}

x86_Allocation *x86_context_allocate_to_gpr(x86_Context *restrict x64_context,
                                            Local  *local,
                                            x86_GPR gpr,
                                            u64     Idx) {
    assert(x64_context != nullptr);
    return x86_allocator_allocate_to_gpr(
        current_allocator(x64_context),
        local,
        gpr,
        Idx,
        x86_context_current_x86_bc(x64_context));
}

x86_Allocation *x86_context_allocate_to_stack(x86_Context *x64_context,
                                              Local       *local,
                                              i64          offset) {
    assert(x64_context != nullptr);
    return x86_allocator_allocate_to_stack(
        current_allocator(x64_context), offset, local);
}

x86_Allocation *x86_context_allocate_result(x86_Context *x64_context,
                                            x86_Location location,
                                            Type        *type) {
    assert(x64_context != nullptr);
    return x86_allocator_allocate_result(
        current_allocator(x64_context), location, type);
}

void x86_context_reallocate_active(x86_Context    *x64_context,
                                   x86_Allocation *active) {
    x86_allocator_reallocate_active(current_allocator(x64_context),
                                    active,
                                    x86_context_current_x86_bc(x64_context));
}

x86_GPR
x86_context_aquire_any_gpr(x86_Context *x64_context, u64 size, u64 Idx) {
    return x86_allocator_aquire_any_gpr(
        current_allocator(x64_context),
        size,
        Idx,
        x86_context_current_x86_bc(x64_context));
}

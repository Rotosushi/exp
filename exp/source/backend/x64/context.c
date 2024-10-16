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

x64_Context x64_context_create(Context *restrict context) {
  assert(context != NULL);
  x64_Context x64_context = {.context  = context,
                             .body     = NULL,
                             .x64_body = NULL,
                             .symbols  = x64_symbol_table_create()};
  return x64_context;
}

void x64_context_destroy(x64_Context *restrict x64_context) {
  assert(x64_context != NULL);
  x64_symbol_table_destroy(&x64_context->symbols);
}

Value *x64_context_value_at(x64_Context *restrict context, u64 index) {
  assert(context != NULL);
  return context_values_at(context->context, index);
}

StringView x64_context_global_labels_at(x64_Context *restrict x64_context,
                                        u64 idx) {
  assert(x64_context != NULL);
  return context_global_labels_at(x64_context->context, idx);
}

void x64_context_enter_function(x64_Context *restrict x64_context,
                                StringView name) {
  assert(x64_context != NULL);
  x64_context->body     = context_enter_function(x64_context->context, name);
  x64_Symbol *symbol    = x64_symbol_table_at(&x64_context->symbols, name);
  x64_context->x64_body = &symbol->body;
  *x64_context->x64_body =
      x64_function_body_create(x64_context->body, x64_context);
}

void x64_context_leave_function(x64_Context *restrict x64_context) {
  assert(x64_context != NULL);
  context_leave_function(x64_context->context);
  x64_context->body     = NULL;
  x64_context->x64_body = NULL;
}

ActualArgumentList *x64_context_call_at(x64_Context *restrict x64_context,
                                        u64 idx) {
  assert(x64_context != NULL);
  return context_call_at(x64_context->context, idx);
}

FormalArgument *x64_context_argument_at(x64_Context *restrict x64_context,
                                        u8 index) {
  assert(x64_context != NULL);
  return context_argument_at(x64_context->context, index);
}

FunctionBody *current_body(x64_Context *restrict x64_context) {
  assert(x64_context->body != NULL);
  return x64_context->body;
}

Bytecode *current_bc(x64_Context *restrict x64_context) {
  return &current_body(x64_context)->bc;
}

LocalVariables *current_locals(x64_Context *restrict x64_context) {
  return &current_body(x64_context)->locals;
}

x64_FunctionBody *current_x64_body(x64_Context *restrict x64_context) {
  assert(x64_context->x64_body != NULL);
  return x64_context->x64_body;
}

x64_Bytecode *current_x64_bc(x64_Context *restrict x64_context) {
  assert(x64_context->x64_body != NULL);
  return &current_x64_body(x64_context)->bc;
}

x64_Allocator *current_allocator(x64_Context *restrict x64_context) {
  return &current_x64_body(x64_context)->allocator;
}

u64 x64_context_current_offset(x64_Context *restrict x64_context) {
  return x64_bytecode_current_offset(current_x64_bc(x64_context));
}

void x64_context_insert(x64_Context *restrict x64_context,
                        x64_Instruction I,
                        u64 offset) {
  x64_bytecode_insert(current_x64_bc(x64_context), I, offset);
}

void x64_context_prepend(x64_Context *restrict x64_context, x64_Instruction I) {
  x64_bytecode_prepend(current_x64_bc(x64_context), I);
}
void x64_context_append(x64_Context *restrict x64_context, x64_Instruction I) {
  x64_bytecode_append(current_x64_bc(x64_context), I);
}

LocalVariable *x64_context_lookup_ssa(x64_Context *restrict x64_context,
                                      u64 ssa) {
  assert(x64_context != NULL);
  return local_variables_lookup_ssa(current_locals(x64_context), ssa);
}

bool x64_context_uses_stack(x64_Context *restrict x64_context) {
  assert(x64_context != NULL);
  return x64_allocator_uses_stack(current_allocator(x64_context));
}

i64 x64_context_stack_size(x64_Context *restrict x64_context) {
  assert(x64_context != NULL);
  return x64_allocator_total_stack_size(current_allocator(x64_context));
}

x64_Allocation *x64_context_allocation_of(x64_Context *restrict x64_context,
                                          u64 ssa) {
  assert(x64_context != NULL);
  return x64_allocator_allocation_of(current_allocator(x64_context), ssa);
}

void x64_context_release_gpr(x64_Context *restrict x64_context,
                             x64_GPR gpr,
                             u64 Idx) {
  assert(x64_context != NULL);
  x64_allocator_release_gpr(
      current_allocator(x64_context), gpr, Idx, current_x64_bc(x64_context));
}

void x64_context_aquire_gpr(x64_Context *restrict x64_context,
                            x64_GPR gpr,
                            u64 Idx) {
  assert(x64_context != NULL);
  x64_allocator_aquire_gpr(
      current_allocator(x64_context), gpr, Idx, current_x64_bc(x64_context));
}

x64_Allocation *x64_context_allocate(x64_Context *restrict x64_context,
                                     LocalVariable *restrict local,
                                     u64 Idx) {
  return x64_allocator_allocate(
      current_allocator(x64_context), Idx, local, current_x64_bc(x64_context));
}

x64_Allocation *
x64_context_allocate_from_active(x64_Context *restrict x64_context,
                                 LocalVariable *local,
                                 x64_Allocation *active,
                                 u64 Idx) {
  return x64_allocator_allocate_from_active(current_allocator(x64_context),
                                            Idx,
                                            local,
                                            active,
                                            current_x64_bc(x64_context));
}

x64_Allocation *x64_context_allocate_to_gpr(x64_Context *restrict x64_context,
                                            LocalVariable *local,
                                            x64_GPR gpr,
                                            u64 Idx) {
  assert(x64_context != NULL);
  return x64_allocator_allocate_to_gpr(current_allocator(x64_context),
                                       gpr,
                                       Idx,
                                       local,
                                       current_x64_bc(x64_context));
}

x64_Allocation *x64_context_allocate_to_stack(x64_Context *restrict x64_context,
                                              LocalVariable *local,
                                              i64 offset) {
  assert(x64_context != NULL);
  return x64_allocator_allocate_to_stack(
      current_allocator(x64_context), offset, local);
}

x64_Allocation *x64_context_allocate_result(x64_Context *restrict x64_context,
                                            x64_Location location,
                                            Type *restrict type) {
  assert(x64_context != NULL);
  return x64_allocator_allocate_result(
      current_allocator(x64_context), location, type);
}

void x64_context_reallocate_active(x64_Context *restrict x64_context,
                                   x64_Allocation *restrict active) {
  x64_allocator_reallocate_active(
      current_allocator(x64_context), active, current_x64_bc(x64_context));
}

x64_GPR x64_context_aquire_any_gpr(x64_Context *restrict x64_context, u64 Idx) {
  return x64_allocator_aquire_any_gpr(
      current_allocator(x64_context), Idx, current_x64_bc(x64_context));
}

x64_Symbol *x64_context_symbol(x64_Context *restrict x64_context,
                               StringView name) {
  assert(x64_context != NULL);
  return x64_symbol_table_at(&x64_context->symbols, name);
}

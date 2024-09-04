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
  x64_Context x64_context = {
      .context = context,
      .symbols = x64_symbol_table_create(context->global_symbol_table.count)};
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

StringView x64_context_global_symbols_at(x64_Context *restrict x64_context,
                                         u64 idx) {
  assert(x64_context != NULL);
  return context_global_labels_at(x64_context->context, idx);
}

FunctionBody *x64_context_enter_function(x64_Context *restrict x64_context,
                                         StringView name) {
  assert(x64_context != NULL);
  return context_enter_function(x64_context->context, name);
}

void x64_context_leave_function(x64_Context *restrict x64_context) {
  assert(x64_context != NULL);
  context_leave_function(x64_context->context);
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

x64_Symbol *x64_context_symbol(x64_Context *restrict x64_context,
                               StringView name) {
  assert(x64_context != NULL);
  return x64_symbol_table_at(&x64_context->symbols, name);
}

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

x64_Context x64_context_create(Context *restrict context) {
  assert(context != NULL);
  x64_Context x64context = {
      .context = context,
      .symbols = x64_symbol_table_create(context->global_symbol_table.count)};
  return x64context;
}

void x64_context_destroy(x64_Context *restrict context) {
  assert(context != NULL);
  x64_symbol_table_destroy(&context->symbols);
}

StringView x64_context_global_symbols_at(x64_Context *restrict context,
                                         u16 idx) {
  assert(context != NULL);
  return context_global_symbols_at(context->context, idx);
}

FunctionBody *x64_context_enter_function(x64_Context *restrict context,
                                         StringView name) {
  assert(context != NULL);
  return context_enter_function(context->context, name);
}

void x64_context_leave_function(x64_Context *restrict context) {
  assert(context != NULL);
  context_leave_function(context->context);
}

ActualArgumentList *x64_context_call_at(x64_Context *restrict context,
                                        u16 idx) {
  assert(context != NULL);
  return context_call_at(context->context, idx);
}

x64_Symbol *x64_context_symbol(x64_Context *restrict context, StringView name) {
  assert(context != NULL);
  return x64_symbol_table_at(&context->symbols, name);
}
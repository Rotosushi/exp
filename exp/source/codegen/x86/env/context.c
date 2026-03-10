/**
 * Copyright (C) 2025 Cade Weinberg
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

#include "codegen/x86/env/context.h"
#include "support/allocation.h"
#include "support/assert.h"

x86_Context *x86_context_allocate() {
    x86_Context *context = callocate(1, sizeof(x86_Context));
    EXP_ASSERT(context != NULL);
    x86_symbol_table_create(&context->symbols);
    return context;
}

void x86_context_deallocate(x86_Context *restrict context) {
    EXP_ASSERT(context != NULL);
    x86_symbol_table_destroy(&context->symbols);
    deallocate(context);
}

x86_Symbol *x86_context_symbol_table_at(x86_Context *context, StringView name) {
    EXP_ASSERT(context != NULL);
    return x86_symbol_table_at(&context->symbols, name);
}

Type const *x86_context_type_nil(x86_Context *restrict context) {
    return context_type_nil(context->parent);
}

Type const *x86_context_type_bool(x86_Context *restrict context) {
    return context_type_bool(context->parent);
}

Type const *x86_context_type_u8(x86_Context *restrict context) {
    return context_type_u8(context->parent);
}

Type const *x86_context_type_u16(x86_Context *restrict context) {
    return context_type_u16(context->parent);
}

Type const *x86_context_type_u32(x86_Context *restrict context) {
    return context_type_u32(context->parent);
}

Type const *x86_context_type_u64(x86_Context *restrict context) {
    return context_type_u64(context->parent);
}

Type const *x86_context_type_i8(x86_Context *restrict context) {
    return context_type_i8(context->parent);
}

Type const *x86_context_type_i16(x86_Context *restrict context) {
    return context_type_i16(context->parent);
}

Type const *x86_context_type_i32(x86_Context *restrict context) {
    return context_type_i32(context->parent);
}

Type const *x86_context_type_i64(x86_Context *restrict context) {
    return context_type_i64(context->parent);
}

Type const *x86_context_type_tuple(x86_Context *restrict context,
                                   TypeTuple tuple) {
    return context_type_tuple(context->parent, tuple);
}

Type const *x86_context_type_function(x86_Context *restrict context,
                                      Type const *argument,
                                      Type const *result) {
    return context_type_function(context->parent, argument, result);
}

x86_Value const *
x86_context_constant_uninitialized(x86_Context *restrict x86_context,
                                   Type const *type);
x86_Value const *x86_context_constant_nil(x86_Context *restrict x86_context);
x86_Value const *x86_context_constant_true(x86_Context *restrict x86_context);
x86_Value const *x86_context_constant_false(x86_Context *restrict x86_context);
x86_Value const *x86_context_constant_u8(x86_Context *restrict x86_context,
                                         u8 u8_);
x86_Value const *x86_context_constant_u16(x86_Context *restrict x86_context,
                                          u16 u16_);
x86_Value const *x86_context_constant_u32(x86_Context *restrict x86_context,
                                          u32 u32_);
x86_Value const *x86_context_constant_u64(x86_Context *restrict x86_context,
                                          u64 u64_);
x86_Value const *x86_context_constant_i8(x86_Context *restrict x86_context,
                                         i8 i8_);
x86_Value const *x86_context_constant_i16(x86_Context *restrict x86_context,
                                          i16 i16_);
x86_Value const *x86_context_constant_i32(x86_Context *restrict x86_context,
                                          i32 i32_);
x86_Value const *x86_context_constant_i64(x86_Context *restrict x86_context,
                                          i64 i64_);
x86_Value const *x86_context_constant_tuple(x86_Context *restrict x86_context,
                                            x86_Tuple tuple,
                                            x86_Function *restrict function);
x86_Value const *
x86_context_constant_function(x86_Context *restrict x86_context,
                              x86_Function function);

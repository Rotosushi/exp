// Copyright (C) 2026 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.

#include "codegen/x86/env/constants.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

static void x86_constant_list_create(x86_ConstantList *restrict list) {
    list->size     = 0;
    list->capacity = 0;
    list->buffer   = NULL;
}

static void x86_constant_list_destroy(x86_ConstantList *restrict list) {
    for (u32 index = 0; index < list->size; ++index) {
        x86_value_deallocate(list->buffer[index]);
    }

    deallocate(list->buffer);
    x86_constant_list_create(list);
}

static bool x86_constant_list_full(x86_ConstantList const *restrict list) {
    return (list->size + 1) >= list->capacity;
}

static void x86_constant_list_grow(x86_ConstantList *restrict list) {
    Growth_u32 g   = array_growth_u32(list->capacity, sizeof(*list->buffer));
    list->buffer   = reallocate(list->buffer, g.alloc_size);
    list->capacity = g.new_capacity;
}

static x86_Value const *
x86_constant_list_append(x86_ConstantList *restrict list, x86_Value *value) {
    for (u32 index = 0; index < list->size; index++) {
        x86_Value *existing = list->buffer[index];
        if (x86_value_equal(existing, value)) {
            x86_value_deallocate(value);
            return existing;
        }
    }

    if (x86_constant_list_full(list)) { x86_constant_list_grow(list); }

    list->buffer[list->size++] = value;
    return value;
}

void x86_constants_create(x86_Constants *restrict constants,
                          struct x86_Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    constants->nil    = x86_value_allocate_nil(x86_context_type_nil(context));
    constants->true_  = x86_value_allocate_bool(true, context);
    constants->false_ = x86_value_allocate_bool(false, context);
    x86_constant_list_create(&constants->u8_list);
    x86_constant_list_create(&constants->u16_list);
    x86_constant_list_create(&constants->u32_list);
    x86_constant_list_create(&constants->u64_list);
    x86_constant_list_create(&constants->i8_list);
    x86_constant_list_create(&constants->i16_list);
    x86_constant_list_create(&constants->i32_list);
    x86_constant_list_create(&constants->i64_list);
    x86_constant_list_create(&constants->tuple_list);
    x86_constant_list_create(&constants->function_list);
}

void x86_constants_destroy(x86_Constants *restrict constants) {
    EXP_ASSERT(constants != NULL);
    x86_value_deallocate(constants->nil);
    x86_value_deallocate(constants->true_);
    x86_value_deallocate(constants->false_);
    x86_constant_list_destroy(&constants->u8_list);
    x86_constant_list_destroy(&constants->u16_list);
    x86_constant_list_destroy(&constants->u32_list);
    x86_constant_list_destroy(&constants->u64_list);
    x86_constant_list_destroy(&constants->i8_list);
    x86_constant_list_destroy(&constants->i16_list);
    x86_constant_list_destroy(&constants->i32_list);
    x86_constant_list_destroy(&constants->i64_list);
    x86_constant_list_destroy(&constants->tuple_list);
    x86_constant_list_destroy(&constants->function_list);
}

x86_Value const *x86_constants_nil(x86_Constants *restrict constants) {
    EXP_ASSERT(constants != NULL);
    return constants->nil;
}

x86_Value const *x86_constants_true(x86_Constants *restrict constants) {
    EXP_ASSERT(constants != NULL);
    return constants->true_;
}

x86_Value const *x86_constants_false(x86_Constants *restrict constants) {
    EXP_ASSERT(constants != NULL);
    return constants->false_;
}

x86_Value const *x86_constants_u8(x86_Constants *restrict constants,
                                  u8 u8_,
                                  struct Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(&constants->u8_list,
                                    x86_value_allocate_u8(u8_, context));
}

x86_Value const *x86_constants_u16(x86_Constants *restrict constants,
                                   u16 u16_,
                                   struct Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(&constants->u16_list,
                                    x86_value_allocate_u16(u16_, context));
}

x86_Value const *x86_constants_u32(x86_Constants *restrict constants,
                                   u32 u32_,
                                   struct x86_Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(&constants->u32_list,
                                    x86_value_allocate_u32(u32_, context));
}

x86_Value const *x86_constants_u64(x86_Constants *restrict constants,
                                   u64 u64_,
                                   struct x86_Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(&constants->u64_list,
                                    x86_value_allocate_u64(u64_, context));
}

x86_Value const *x86_constants_i8(x86_Constants *restrict constants,
                                  i8 i8_,
                                  struct x86_Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(&constants->i8_list,
                                    x86_value_allocate_i8(i8_, context));
}

x86_Value const *x86_constants_i16(x86_Constants *restrict constants,
                                   i16 i16_,
                                   struct x86_Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(&constants->i16_list,
                                    x86_value_allocate_i16(i16_, context));
}

x86_Value const *x86_constants_i32(x86_Constants *restrict constants,
                                   i32 i32_,
                                   struct x86_Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(&constants->i32_list,
                                    x86_value_allocate_i32(i32_, context));
}

x86_Value const *x86_constants_i64(x86_Constants *restrict constants,
                                   i64 i64_,
                                   struct x86_Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(&constants->i64_list,
                                    x86_value_allocate_i64(i64_, context));
}

x86_Value const *x86_constants_tuple(x86_Constants *restrict constants,
                                     x86_Tuple tuple,
                                     x86_Function *restrict function,
                                     struct x86_Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(
        &constants->tuple_list,
        x86_value_allocate_tuple(tuple, function, context));
}

x86_Value const *x86_constants_function(x86_Constants *restrict constants,
                                        x86_Function function,
                                        struct x86_Context *restrict context) {
    EXP_ASSERT(constants != NULL);
    EXP_ASSERT(context != NULL);
    return x86_constant_list_append(
        &constants->u8_list, x86_value_allocate_function(function, context));
}

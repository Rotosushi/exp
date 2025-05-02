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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "env/constants.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

static void constant_list_create(ConstantList *restrict list) {
    list->size     = 0;
    list->capacity = 0;
    list->buffer   = NULL;
}

static void constant_list_destroy(ConstantList *restrict list) {
    for (u32 index = 0; index < list->size; ++index) {
        value_deallocate(list->buffer[index]);
    }

    deallocate(list->buffer);
    constant_list_create(list);
}

static bool constant_list_full(ConstantList const *restrict list) {
    return (list->size + 1) >= list->capacity;
}

static void constant_list_grow(ConstantList *restrict list) {
    Growth_u32 g   = array_growth_u32(list->capacity, sizeof(*list->buffer));
    list->buffer   = reallocate(list->buffer, g.alloc_size);
    list->capacity = g.new_capacity;
}

static Value const *constant_list_append(ConstantList *restrict list,
                                         Value *value) {
    for (u32 index = 0; index < list->size; index++) {
        Value *existing = list->buffer[index];
        if (value_equal(existing, value)) {
            value_deallocate(value);
            return existing;
        }
    }

    if (constant_list_full(list)) { constant_list_grow(list); }

    list->buffer[list->size++] = value;
    return value;
}

void constants_create(Constants *restrict constants) {
    exp_assert(constants != NULL);
    constants->nil    = value_allocate_nil();
    constants->true_  = value_allocate_bool(true);
    constants->false_ = value_allocate_bool(false);
    constant_list_create(&constants->u8_list);
    constant_list_create(&constants->u16_list);
    constant_list_create(&constants->u32_list);
    constant_list_create(&constants->u64_list);
    constant_list_create(&constants->i8_list);
    constant_list_create(&constants->i16_list);
    constant_list_create(&constants->i32_list);
    constant_list_create(&constants->i64_list);
    constant_list_create(&constants->tuple_list);
    constant_list_create(&constants->function_list);
}

void constants_destroy(Constants *restrict constants) {
    exp_assert(constants != NULL);
    value_deallocate(constants->nil);
    value_deallocate(constants->true_);
    value_deallocate(constants->false_);
    constant_list_destroy(&constants->u8_list);
    constant_list_destroy(&constants->u16_list);
    constant_list_destroy(&constants->u32_list);
    constant_list_destroy(&constants->u64_list);
    constant_list_destroy(&constants->i8_list);
    constant_list_destroy(&constants->i16_list);
    constant_list_destroy(&constants->i32_list);
    constant_list_destroy(&constants->i64_list);
    constant_list_destroy(&constants->tuple_list);
    constant_list_destroy(&constants->function_list);
}

Value const *constants_nil(Constants *restrict constants) {
    exp_assert(constants != NULL);
    return constants->nil;
}

Value const *constants_true(Constants *restrict constants) {
    exp_assert(constants != NULL);
    return constants->true_;
}

Value const *constants_false(Constants *restrict constants) {
    exp_assert(constants != NULL);
    return constants->false_;
}

Value const *constants_u8(Constants *restrict constants, u8 u8_) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->u8_list, value_allocate_u8(u8_));
}

Value const *constants_u16(Constants *restrict constants, u16 u16_) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->u16_list, value_allocate_u16(u16_));
}

Value const *constants_u32(Constants *restrict constants, u32 u32_) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->u32_list, value_allocate_u32(u32_));
}

Value const *constants_u64(Constants *restrict constants, u64 u64_) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->u64_list, value_allocate_u64(u64_));
}

Value const *constants_i8(Constants *restrict constants, i8 i8_) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->i8_list, value_allocate_i8(i8_));
}

Value const *constants_i16(Constants *restrict constants, i16 i16_) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->i16_list, value_allocate_i16(i16_));
}

Value const *constants_i32(Constants *restrict constants, i32 i32_) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->i32_list, value_allocate_i32(i32_));
}

Value const *constants_i64(Constants *restrict constants, i64 i64_) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->i64_list, value_allocate_i64(i64_));
}

Value const *constants_tuple(Constants *restrict constants, Value *tuple) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->tuple_list, tuple);
}

Value const *constants_function(Constants *restrict constants,
                                Value *function) {
    exp_assert(constants != NULL);
    return constant_list_append(&constants->u8_list, function);
}

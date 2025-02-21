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

/**
 * @file env/type_interner.c
 */
#include "env/type_interner.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"

static void type_list_initialize(TypeList *type_list) {
    EXP_ASSERT(type_list != nullptr);
    type_list->count    = 0;
    type_list->capacity = 0;
    type_list->buffer   = nullptr;
}

static void type_list_termiante(TypeList *type_list) {
    EXP_ASSERT(type_list != nullptr);
    for (u64 i = 0; i < type_list->count; ++i) {
        Type *T = type_list->buffer[i];
        EXP_ASSERT(T != nullptr);
        type_terminate(T);
    }

    deallocate(type_list->buffer);
    type_list->buffer   = nullptr;
    type_list->count    = 0;
    type_list->capacity = 0;
}

static bool type_list_full(TypeList *type_list) {
    EXP_ASSERT(type_list != nullptr);
    return (type_list->count + 1) >= type_list->capacity;
}

static void type_list_grow(TypeList *type_list) {
    Growth32 g          = array_growth_u32(type_list->capacity, sizeof(Type *));
    type_list->buffer   = reallocate(type_list->buffer, g.alloc_size);
    type_list->capacity = g.new_capacity;
}

static Type *type_list_append(TypeList *type_list, Type *T) {
    EXP_ASSERT(type_list != nullptr);
    EXP_ASSERT(T != nullptr);

    for (u32 i = 0; i < type_list->count; ++i) {
        Type *U = type_list->buffer[i];
        if (type_equal(T, U)) {
            type_terminate(T);
            return U;
        }
    }

    if (type_list_full(type_list)) { type_list_grow(type_list); }

    type_list->buffer[type_list->count++] = T;
    return T;
}

void type_interner_initialize(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    type_interner->nil_type     = type_nil();
    type_interner->boolean_type = type_bool();
    type_interner->i8_type      = type_i8();
    type_interner->i16_type     = type_i16();
    type_interner->i32_type     = type_i32();
    type_interner->i64_type     = type_i64();
    type_interner->u8_type      = type_u8();
    type_interner->u16_type     = type_u16();
    type_interner->u32_type     = type_u32();
    type_interner->u64_type     = type_u64();
    type_list_initialize(&type_interner->tuple_types);
    type_list_initialize(&type_interner->function_types);
}

void type_interner_destroy(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    type_terminate(type_interner->nil_type);
    type_terminate(type_interner->boolean_type);
    type_terminate(type_interner->i8_type);
    type_terminate(type_interner->i16_type);
    type_terminate(type_interner->i32_type);
    type_terminate(type_interner->i64_type);
    type_terminate(type_interner->u8_type);
    type_terminate(type_interner->u16_type);
    type_terminate(type_interner->u32_type);
    type_terminate(type_interner->u64_type);
    type_list_termiante(&type_interner->tuple_types);
    type_list_termiante(&type_interner->function_types);
}

Type const *type_interner_nil_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->nil_type;
}

Type const *type_interner_boolean_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->boolean_type;
}

Type const *type_interner_i8_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->i8_type;
}

Type const *type_interner_i16_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->i16_type;
}

Type const *type_interner_i32_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->i32_type;
}

Type const *type_interner_i64_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->i64_type;
}

Type const *type_interner_u8_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->u8_type;
}

Type const *type_interner_u16_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->u16_type;
}

Type const *type_interner_u32_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->u32_type;
}

Type const *type_interner_u64_type(TypeInterner *type_interner) {
    EXP_ASSERT(type_interner != nullptr);
    return type_interner->u64_type;
}

Type const *type_interner_tuple_type(TypeInterner *type_interner,
                                     TupleType tuple) {
    EXP_ASSERT(type_interner != nullptr);
    Type *T = type_tuple(tuple);
    return type_list_append(&type_interner->tuple_types, T);
}

Type const *type_interner_function_type(TypeInterner *type_interner,
                                        Type const *return_type,
                                        TupleType argument_types) {
    EXP_ASSERT(type_interner != nullptr);
    EXP_ASSERT(return_type != nullptr);
    Type *T = type_function(return_type, argument_types);
    return type_list_append(&type_interner->function_types, T);
}

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
#include <assert.h>
#include <stdlib.h>

#include "env/type_interner.h"
#include "support/allocation.h"
#include "support/array_growth.h"

static void type_list_create(TypeList *restrict type_list) {
    assert(type_list != NULL);
    type_list->size     = 0;
    type_list->capacity = 0;
    type_list->buffer   = NULL;
}

static void type_list_destroy(TypeList *restrict type_list) {
    assert(type_list != NULL);

    for (u32 index = 0; index < type_list->size; ++index) {
        deallocate(type_list->buffer[index]);
    }

    type_list_create(type_list);
}

static bool type_list_full(TypeList const *restrict type_list) {
    assert(type_list != NULL);
    return (type_list->size + 1) >= type_list->capacity;
}

static void type_list_grow(TypeList *restrict type_list) {
    assert(type_list != NULL);
    Growth_u32 g =
        array_growth_u32(type_list->capacity, sizeof(*type_list->buffer));
    type_list->buffer   = reallocate(type_list->buffer, g.alloc_size);
    type_list->capacity = g.new_capacity;
}

static void type_list_append(TypeList *restrict type_list,
                             Type *restrict type) {
    assert(type_list != NULL);
    assert(type != NULL);
    if (type_list_full(type_list)) { type_list_grow(type_list); }
    type_list->buffer[type_list->size++] = type;
}

TypeInterner type_interner_create() {
    TypeInterner type_interner;
    type_interner.nil_type     = type_create_nil();
    type_interner.boolean_type = type_create_boolean();
    type_interner.u8_type      = type_create_u8();
    type_interner.u16_type     = type_create_u16();
    type_interner.u32_type     = type_create_u32();
    type_interner.u64_type     = type_create_u64();
    type_interner.i8_type      = type_create_i8();
    type_interner.i16_type     = type_create_i16();
    type_interner.i32_type     = type_create_i32();
    type_interner.i64_type     = type_create_i64();
    type_list_create(&type_interner.tuple_types);
    type_list_create(&type_interner.function_types);
    return type_interner;
}

void type_interner_destroy(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    type_list_destroy(&type_interner->tuple_types);
    type_list_destroy(&type_interner->function_types);
    return;
}

Type const *type_interner_nil_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->nil_type);
}

Type const *type_interner_boolean_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->boolean_type);
}

Type const *type_interner_u8_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->u8_type);
}

Type const *type_interner_u16_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->u16_type);
}

Type const *type_interner_u32_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->u32_type);
}

Type const *type_interner_u64_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->u64_type);
}

Type const *type_interner_i8_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->i8_type);
}

Type const *type_interner_i16_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->i16_type);
}

Type const *type_interner_i32_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->i32_type);
}

Type const *type_interner_i64_type(TypeInterner *restrict type_interner) {
    assert(type_interner != NULL);
    return &(type_interner->i64_type);
}

Type const *type_interner_tuple_type(TypeInterner *restrict type_interner,
                                     TupleType tuple) {
    assert(type_interner != NULL);
    Type *type = allocate(sizeof(Type));
    *type      = type_create_tuple(tuple);
    type_list_append(&type_interner->tuple_types, type);
    return type;
}

Type const *type_interner_function_type(TypeInterner *restrict type_interner,
                                        Type const *return_type,
                                        TupleType   argument_types) {
    assert(type_interner != NULL);
    assert(return_type != NULL);
    Type *type = allocate(sizeof(Type));
    *type      = type_create_function(return_type, argument_types);
    type_list_append(&type_interner->function_types, type);
    return type;
}

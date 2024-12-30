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
#include "utility/alloc.h"
#include "utility/array_growth.h"

static void type_list_initialize(TypeList *type_list) {
    assert(type_list != nullptr);
    type_list->count    = 0;
    type_list->capacity = 0;
    type_list->buffer   = nullptr;
}

static void type_list_termiante(TypeList *type_list) {
    assert(type_list != nullptr);
    for (u64 i = 0; i < type_list->count; ++i) {
        Type *T = type_list->buffer[i];
        assert(T != nullptr);
        type_terminate(T);
    }

    deallocate(type_list->buffer);
    type_list->buffer   = nullptr;
    type_list->count    = 0;
    type_list->capacity = 0;
}

static bool type_list_full(TypeList *type_list) {
    assert(type_list != nullptr);
    return (type_list->count + 1) >= type_list->capacity;
}

static void type_list_grow(TypeList *type_list) {
    Growth64 g          = array_growth_u64(type_list->capacity, sizeof(Type *));
    type_list->buffer   = reallocate(type_list->buffer, g.alloc_size);
    type_list->capacity = g.new_capacity;
}

static Type *type_list_append(TypeList *type_list, Type *T) {
    assert(type_list != nullptr);
    assert(T != nullptr);

    for (u64 i = 0; i < type_list->count; ++i) {
        Type *U = type_list->buffer[i];
        if (type_equality(T, U)) {
            type_terminate(T);
            return U;
        }
    }

    if (type_list_full(type_list)) { type_list_grow(type_list); }

    type_list->buffer[type_list->count++] = T;
    return T;
}

void type_interner_initialize(TypeInterner *type_interner) {
    assert(type_interner != nullptr);
    // type_interner->nil_type     = type_nil();
    // type_interner->boolean_type = type_boolean();
    type_interner->i64_type = type_i64();
    type_list_initialize(&type_interner->tuple_types);
    type_list_initialize(&type_interner->function_types);
}

void type_interner_destroy(TypeInterner *type_interner) {
    assert(type_interner != NULL);
    // type_terminate(type_interner->nil_type);
    // type_terminate(type_interner->boolean_type);
    type_terminate(type_interner->i64_type);
    type_list_termiante(&type_interner->tuple_types);
    type_list_termiante(&type_interner->function_types);
}

/*
Type const *type_interner_nil_type(TypeInterner *type_interner) {
    assert(type_interner != NULL);
    return type_interner->nil_type;
}

Type const *type_interner_boolean_type(TypeInterner *type_interner) {
    assert(type_interner != NULL);
    return type_interner->boolean_type;
}
*/

Type const *type_interner_i64_type(TypeInterner *type_interner) {
    assert(type_interner != NULL);
    return type_interner->i64_type;
}

Type const *type_interner_tuple_type(TypeInterner *type_interner,
                                     TupleType tuple) {
    assert(type_interner != NULL);
    Type *T = type_tuple(tuple);
    return type_list_append(&type_interner->tuple_types, T);
}

Type const *type_interner_function_type(TypeInterner *type_interner,
                                        Type const *return_type,
                                        TupleType argument_types) {
    assert(type_interner != NULL);
    Type *T = type_function(return_type, argument_types);
    return type_list_append(&type_interner->function_types, T);
}

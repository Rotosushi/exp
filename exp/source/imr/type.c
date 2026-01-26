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
#include <stdlib.h>

#include "env/context.h"
#include "imr/type.h"
#include "support/assert.h"
#include "support/unreachable.h"

static void type_create_primary(Type *restrict type,
                                TypePrimary primary,
                                Context *restrict context) {
    EXP_ASSERT(type != NULL);
    type->kind    = TYPE_KIND_PRIMARY;
    type->primary = primary;
    type->layout  = context_layout_of(context, type);
}

static void type_create_composite(Type *restrict type,
                                  TypeComposite composite,
                                  Context *restrict context) {
    EXP_ASSERT(type != NULL);
    type->kind      = TYPE_KIND_COMPOSITE;
    type->composite = composite;
    type->layout    = context_layout_of(context, type);
}

void type_create_nil(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_nil(), context);
}

void type_create_bool(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_bool(), context);
}

void type_create_u8(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_u8(), context);
}

void type_create_u16(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_u16(), context);
}

void type_create_u32(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_u32(), context);
}

void type_create_u64(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_u64(), context);
}

void type_create_i8(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_i8(), context);
}

void type_create_i16(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_i16(), context);
}

void type_create_i32(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_i32(), context);
}

void type_create_i64(Type *restrict type, Context *restrict context) {
    type_create_primary(type, type_primary_i64(), context);
}

void type_create_tuple(Type *restrict type,
                       TypeTuple tuple,
                       Context *restrict context) {
    type_create_composite(type, type_composite_tuple(tuple), context);
}

void type_create_function(Type *restrict type,
                          Type const *argument,
                          Type const *result,
                          Context *restrict context) {
    type_create_composite(
        type, type_composite_function(argument, result), context);
}

void type_destroy(Type *restrict type) {
    EXP_ASSERT(type != NULL);
    switch (type->kind) {
    case TYPE_KIND_COMPOSITE: type_composite_destroy(&type->composite); break;

    // #NOTE: no primary types dynamically allocate
    default: break;
    }
}

bool type_equality(Type const *A, Type const *B) {
    EXP_ASSERT(A != NULL);
    EXP_ASSERT(B != NULL);
    if (A->kind != B->kind) { return 0; }

    switch (A->kind) {
    case TYPE_KIND_PRIMARY:
        return type_primary_equality(A->primary, B->primary);

    case TYPE_KIND_COMPOSITE:
        return type_composite_equality(&A->composite, &B->composite);

    // #NOTE: scalar types are equal when their kinds are equal
    default: return true;
    }
}

bool type_is_primary(Type const *restrict type) {
    EXP_ASSERT(type != NULL);
    return type->kind == TYPE_KIND_PRIMARY;
}

bool type_is_composite(Type const *restrict type) {
    EXP_ASSERT(type != NULL);
    return type->kind == TYPE_KIND_COMPOSITE;
}

bool type_is_integral(Type const *restrict type) {
    EXP_ASSERT(type != NULL);
    if (!type_is_primary(type)) { return false; }
    return type_primary_is_integral(type->primary);
}

bool type_is_callable(Type const *restrict type) {
    EXP_ASSERT(type != NULL);
    if (!type_is_composite(type)) { return false; }
    return type_composite_is_callable(&type->composite);
}

bool type_is_index(Type const *restrict T) {
    EXP_ASSERT(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_PRIMARY: return type_primary_is_index(T->primary);

    default: return false;
    }
}

bool type_is_indexable(Type const *restrict T) {
    EXP_ASSERT(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_COMPOSITE: return type_composite_is_indexable(&T->composite);

    default: return false;
    }
}

void print_type(String *restrict string, Type const *restrict T) {
    EXP_ASSERT(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_PRIMARY: print_type_primary(string, T->primary); break;
    case TYPE_KIND_COMPOSITE:
        print_type_composite(string, &T->composite);
        break;

    default: EXP_UNREACHABLE();
    }
}

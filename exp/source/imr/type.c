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

#include "imr/type.h"
#include "support/assert.h"
#include "support/unreachable.h"

static void type_create_primary(Type *restrict type, TypePrimary primary) {
    exp_assert(type != NULL);
    type->kind    = TYPE_KIND_PRIMARY;
    type->primary = primary;
}

static void type_create_composite(Type *restrict type,
                                  TypeComposite composite) {
    exp_assert(type != NULL);
    type->kind      = TYPE_KIND_COMPOSITE;
    type->composite = composite;
}

void type_create_nil(Type *restrict type) {
    type_create_primary(type, type_primary_nil());
}

void type_create_bool(Type *restrict type) {
    type_create_primary(type, type_primary_bool());
}

void type_create_u8(Type *restrict type) {
    type_create_primary(type, type_primary_u8());
}

void type_create_u16(Type *restrict type) {
    type_create_primary(type, type_primary_u16());
}

void type_create_u32(Type *restrict type) {
    type_create_primary(type, type_primary_u32());
}

void type_create_u64(Type *restrict type) {
    type_create_primary(type, type_primary_u64());
}

void type_create_i8(Type *restrict type) {
    type_create_primary(type, type_primary_i8());
}

void type_create_i16(Type *restrict type) {
    type_create_primary(type, type_primary_i16());
}

void type_create_i32(Type *restrict type) {
    type_create_primary(type, type_primary_i32());
}

void type_create_i64(Type *restrict type) {
    type_create_primary(type, type_primary_i64());
}

void type_create_tuple(Type *restrict type, TypeTuple tuple) {
    type_create_composite(type, type_composite_tuple(tuple));
}

void type_create_function(Type *restrict type,
                          Type const *result,
                          TypeTuple   args) {
    type_create_composite(type, type_composite_function(result, args));
}

void type_destroy(Type *restrict type) {
    exp_assert(type != NULL);
    switch (type->kind) {
    case TYPE_KIND_COMPOSITE: type_composite_destroy(&type->composite); break;

    // #NOTE: no primary types dynamically allocate
    default: break;
    }
}

bool type_equality(Type const *A, Type const *B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
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
    exp_assert(type != NULL);
    return type->kind == TYPE_KIND_PRIMARY;
}

bool type_is_composite(Type const *restrict type) {
    exp_assert(type != NULL);
    return type->kind == TYPE_KIND_COMPOSITE;
}

bool type_is_integral(Type const *restrict type) {
    exp_assert(type != NULL);
    if (!type_is_primary(type)) { return false; }
    return type_primary_is_integral(type->primary);
}

bool type_is_callable(Type const *restrict type) {
    exp_assert(type != NULL);
    if (!type_is_composite(type)) { return false; }
    return type_composite_is_callable(&type->composite);
}

bool type_is_index(Type const *restrict T) {
    exp_assert(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_PRIMARY: return type_primary_is_index(T->primary);

    default: return false;
    }
}

bool type_is_indexable(Type const *restrict T) {
    exp_assert(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_COMPOSITE: return type_composite_is_indexable(&T->composite);

    default: return false;
    }
}

void print_type(String *restrict string, Type const *restrict T) {
    exp_assert(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_PRIMARY:   print_type_primary(string, T->primary);
    case TYPE_KIND_COMPOSITE: print_type_composite(string, &T->composite);

    default: EXP_UNREACHABLE();
    }
}

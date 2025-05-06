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
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"
#include "support/unreachable.h"

TupleType tuple_type_create() {
    TupleType tuple_type;
    tuple_type.capacity = 0;
    tuple_type.size     = 0;
    tuple_type.types    = NULL;
    return tuple_type;
}

void tuple_type_destroy(TupleType *restrict tuple_type) {
    exp_assert(tuple_type != NULL);
    tuple_type->capacity = 0;
    tuple_type->size     = 0;
    deallocate(tuple_type->types);
    tuple_type->types = NULL;
}

bool tuple_type_equal(TupleType const *A, TupleType const *B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
    if (A == B) { return 1; }

    if (A->size != B->size) { return 0; }

    for (u64 i = 0; i < A->size; ++i) {
        Type const *t = A->types[i];
        Type const *u = B->types[i];

        if (!type_equality(t, u)) { return 0; }
    }

    return 1;
}

static bool tuple_type_full(TupleType *restrict tuple_type) {
    return (tuple_type->size + 1) >= tuple_type->capacity;
}

static void tuple_type_grow(TupleType *restrict tuple_type) {
    Growth_u32 g      = array_growth_u32(tuple_type->capacity, sizeof(Type *));
    tuple_type->types = reallocate(tuple_type->types, g.alloc_size);
    tuple_type->capacity = g.new_capacity;
}

void tuple_type_append(TupleType *restrict tuple_type, Type const *type) {
    exp_assert(tuple_type != NULL);

    if (tuple_type_full(tuple_type)) { tuple_type_grow(tuple_type); }

    tuple_type->types[tuple_type->size] = type;
    tuple_type->size += 1;
}

bool function_type_equal(FunctionType const *A, FunctionType const *B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
    if (A == B) { return 1; }

    if (!type_equality(A->return_type, B->return_type)) { return 0; }

    return tuple_type_equal(&A->argument_types, &B->argument_types);
}

Type type_create_nil() {
    return (Type){.kind = TYPE_KIND_NIL, .scalar_type = 0};
}

Type type_create_boolean() {
    return (Type){.kind = TYPE_KIND_BOOLEAN, .scalar_type = 0};
}

Type type_create_u8() { return (Type){.kind = TYPE_KIND_U8, .scalar_type = 0}; }

Type type_create_u16() {
    return (Type){.kind = TYPE_KIND_U16, .scalar_type = 0};
}

Type type_create_u32() {
    return (Type){.kind = TYPE_KIND_U32, .scalar_type = 0};
}

Type type_create_u64() {
    return (Type){.kind = TYPE_KIND_U64, .scalar_type = 0};
}

Type type_create_i8() { return (Type){.kind = TYPE_KIND_I8, .scalar_type = 0}; }

Type type_create_i16() {
    return (Type){.kind = TYPE_KIND_I16, .scalar_type = 0};
}

Type type_create_i32() {
    return (Type){.kind = TYPE_KIND_I32, .scalar_type = 0};
}

Type type_create_i64() {
    return (Type){.kind = TYPE_KIND_I64, .scalar_type = 0};
}

Type type_create_tuple(TupleType tuple) {
    return (Type){.kind = TYPE_KIND_TUPLE, .tuple_type = tuple};
}

Type type_create_function(Type const *result, TupleType args) {
    return (Type){
        .kind          = TYPE_KIND_FUNCTION,
        .function_type = (FunctionType){result, args}
    };
}

void type_destroy(Type *type) {
    switch (type->kind) {
    case TYPE_KIND_TUPLE: {
        tuple_type_destroy(&type->tuple_type);
        break;
    }

    case TYPE_KIND_FUNCTION: {
        tuple_type_destroy(&type->function_type.argument_types);
        break;
    }

    // #NOTE: no other types dynamically allocate
    default: break;
    }
}

bool type_equality(Type const *A, Type const *B) {
    if (A->kind != B->kind) { return 0; }

    switch (A->kind) {
    case TYPE_KIND_TUPLE:
        return tuple_type_equal(&A->tuple_type, &B->tuple_type);
    case TYPE_KIND_FUNCTION:
        return function_type_equal(&A->function_type, &B->function_type);

    // #NOTE: scalar types are equal when their kinds are equal
    default: return true;
    }
}

bool type_is_scalar(Type const *T) {
    switch (T->kind) {
    case TYPE_KIND_NIL:
    case TYPE_KIND_BOOLEAN:
    case TYPE_KIND_U8:
    case TYPE_KIND_U16:
    case TYPE_KIND_U32:
    case TYPE_KIND_U64:
    case TYPE_KIND_I8:
    case TYPE_KIND_I16:
    case TYPE_KIND_I32:
    case TYPE_KIND_I64:     return true;

    // a tuple type of size two or more cannot be scalar
    // unless we optimize it to be so. which is a TODO.
    case TYPE_KIND_TUPLE:
    default:              return false;
    }
}

bool type_is_index(Type const *T) {
    switch (T->kind) {
    case TYPE_KIND_U8:
    case TYPE_KIND_U16:
    case TYPE_KIND_U32:
    case TYPE_KIND_U64:
    case TYPE_KIND_I8:
    case TYPE_KIND_I16:
    case TYPE_KIND_I32:
    case TYPE_KIND_I64: return true;

    default: return false;
    }
}

bool type_is_callable(Type const *T) {
    switch (T->kind) {
    case TYPE_KIND_FUNCTION: return true;
    default:                 return false;
    }
}

bool type_is_indexable(Type const *T) {
    switch (T->kind) {
    case TYPE_KIND_TUPLE: return true;
    default:              return false;
    }
}

static void print_tuple_type(String *restrict string,
                             TupleType const *restrict tuple_type) {
    string_append(string, SV("("));
    for (u64 i = 0; i < tuple_type->size; ++i) {
        print_type(string, tuple_type->types[i]);

        if (i < (tuple_type->size - 1)) { string_append(string, SV(", ")); }
    }
    string_append(string, SV(")"));
}

static void print_function_type(String *restrict string,
                                FunctionType const *restrict function_type) {
    string_append(string, SV("fn "));
    TupleType const *tuple_type = &function_type->argument_types;
    print_tuple_type(string, tuple_type);
    string_append(string, SV(" -> "));
    print_type(string, function_type->return_type);
}

void print_type(String *restrict string, Type const *restrict T) {
    switch (T->kind) {
    case TYPE_KIND_NIL:     string_append(string, SV("nil")); break;
    case TYPE_KIND_BOOLEAN: string_append(string, SV("bool")); break;
    case TYPE_KIND_I64:     string_append(string, SV("i64")); break;
    case TYPE_KIND_TUPLE:   print_tuple_type(string, &T->tuple_type); break;
    case TYPE_KIND_FUNCTION:
        print_function_type(string, &T->function_type);
        break;

    default: EXP_UNREACHABLE();
    }
}

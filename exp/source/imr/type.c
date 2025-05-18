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

void tuple_type_create(TupleType *restrict tuple_type) {
    exp_assert(tuple_type != NULL);
    tuple_type->capacity = 0;
    tuple_type->length   = 0;
    tuple_type->types    = NULL;
}

void tuple_type_destroy(TupleType *restrict tuple_type) {
    exp_assert(tuple_type != NULL);
    tuple_type->capacity = 0;
    tuple_type->length   = 0;
    deallocate(tuple_type->types);
    tuple_type->types = NULL;
}

bool tuple_type_equal(TupleType const *A, TupleType const *B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
    if (A == B) { return 1; }

    if (A->length != B->length) { return 0; }

    for (u64 i = 0; i < A->length; ++i) {
        Type const *t = A->types[i];
        Type const *u = B->types[i];

        if (!type_equality(t, u)) { return 0; }
    }

    return 1;
}

bool tuple_type_index_in_bounds(TupleType const *restrict tuple, u32 index) {
    exp_assert(tuple != NULL);
    return tuple->length > index;
}

static bool tuple_type_full(TupleType *restrict tuple_type) {
    return (tuple_type->length + 1) >= tuple_type->capacity;
}

static void tuple_type_grow(TupleType *restrict tuple_type) {
    Growth_u32 g      = array_growth_u32(tuple_type->capacity, sizeof(Type *));
    tuple_type->types = reallocate(tuple_type->types, g.alloc_size);
    tuple_type->capacity = g.new_capacity;
}

void tuple_type_append(TupleType *restrict tuple_type, Type const *type) {
    exp_assert(tuple_type != NULL);

    if (tuple_type_full(tuple_type)) { tuple_type_grow(tuple_type); }

    tuple_type->types[tuple_type->length] = type;
    tuple_type->length += 1;
}

Type const *tuple_type_at(TupleType const *restrict tuple, u32 index) {
    exp_assert(tuple != NULL);
    exp_assert(tuple_type_index_in_bounds(tuple, index));
    return tuple->types[index];
}

bool function_type_equal(FunctionType const *A, FunctionType const *B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
    if (A == B) { return 1; }

    if (!type_equality(A->return_type, B->return_type)) { return 0; }

    return tuple_type_equal(&A->argument_types, &B->argument_types);
}

void type_create_nil(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_NIL, .scalar = 0};
}

void type_create_bool(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_BOOL, .scalar = 0};
}

void type_create_u8(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_U8, .scalar = 0};
}

void type_create_u16(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_U16, .scalar = 0};
}

void type_create_u32(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_U32, .scalar = 0};
}

void type_create_u64(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_U64, .scalar = 0};
}

void type_create_i8(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_I8, .scalar = 0};
}

void type_create_i16(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_I16, .scalar = 0};
}

void type_create_i32(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_I32, .scalar = 0};
}

void type_create_i64(Type *restrict type) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_I64, .scalar = 0};
}

void type_create_tuple(Type *restrict type, TupleType tuple) {
    exp_assert(type != NULL);
    *type = (Type){.kind = TYPE_KIND_TUPLE, .tuple = tuple};
}

void type_create_function(Type *restrict type,
                          Type const *result,
                          TupleType   args) {
    exp_assert(type != NULL);
    *type = (Type){
        .kind = TYPE_KIND_FUNCTION, .function = (FunctionType){result, args}
    };
}

void type_destroy(Type *restrict type) {
    exp_assert(type != NULL);
    switch (type->kind) {
    case TYPE_KIND_TUPLE: {
        tuple_type_destroy(&type->tuple);
        break;
    }

    case TYPE_KIND_FUNCTION: {
        tuple_type_destroy(&type->function.argument_types);
        break;
    }

    // #NOTE: no other types dynamically allocate
    default: break;
    }
}

bool type_equality(Type const *A, Type const *B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
    if (A->kind != B->kind) { return 0; }

    switch (A->kind) {
    case TYPE_KIND_TUPLE: return tuple_type_equal(&A->tuple, &B->tuple);
    case TYPE_KIND_FUNCTION:
        return function_type_equal(&A->function, &B->function);

    // #NOTE: scalar types are equal when their kinds are equal
    default: return true;
    }
}

bool type_is_scalar(Type const *T) {
    exp_assert(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_NIL:
    case TYPE_KIND_BOOL:
    case TYPE_KIND_U8:
    case TYPE_KIND_U16:
    case TYPE_KIND_U32:
    case TYPE_KIND_U64:
    case TYPE_KIND_I8:
    case TYPE_KIND_I16:
    case TYPE_KIND_I32:
    case TYPE_KIND_I64:  return true;

    // a tuple type of size two or more cannot be scalar
    // unless we optimize it to be so. which is a TODO.
    case TYPE_KIND_TUPLE:
    default:              return false;
    }
}

bool type_is_index(Type const *T) {
    exp_assert(T != NULL);
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
    exp_assert(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_FUNCTION: return true;
    default:                 return false;
    }
}

bool type_is_indexable(Type const *T) {
    exp_assert(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_TUPLE: return true;
    default:              return false;
    }
}

static void print_tuple_type(String *restrict string,
                             TupleType const *restrict tuple_type) {
    string_append(string, SV("("));
    for (u64 i = 0; i < tuple_type->length; ++i) {
        print_type(string, tuple_type->types[i]);

        if (i < (tuple_type->length - 1)) { string_append(string, SV(", ")); }
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
    exp_assert(T != NULL);
    switch (T->kind) {
    case TYPE_KIND_NIL:      string_append(string, SV("nil")); break;
    case TYPE_KIND_BOOL:     string_append(string, SV("bool")); break;
    case TYPE_KIND_I64:      string_append(string, SV("i64")); break;
    case TYPE_KIND_TUPLE:    print_tuple_type(string, &T->tuple); break;
    case TYPE_KIND_FUNCTION: print_function_type(string, &T->function); break;

    default: EXP_UNREACHABLE();
    }
}

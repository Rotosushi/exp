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
 * @file imr/type.c
 */

#include "imr/type.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

void tuple_type_initialize(TupleType *tuple_type) {
    EXP_ASSERT(tuple_type != nullptr);
    tuple_type->capacity = 0;
    tuple_type->count    = 0;
    tuple_type->types    = nullptr;
}

void tuple_type_terminate(TupleType *tuple_type) {
    EXP_ASSERT(tuple_type != nullptr);
    tuple_type->capacity = 0;
    tuple_type->count    = 0;
    deallocate(tuple_type->types);
    tuple_type->types = nullptr;
}

bool tuple_type_equality(TupleType const *A, TupleType const *B) {
    EXP_ASSERT(A != nullptr);
    EXP_ASSERT(B != nullptr);
    if (A == B) { return true; }

    if (A->count != B->count) { return false; }

    for (u64 i = 0; i < A->count; ++i) {
        Type const *t = A->types[i];
        Type const *u = B->types[i];

        if (!type_equal(t, u)) { return false; }
    }

    return true;
}

static bool tuple_type_full(TupleType *tuple_type) {
    EXP_ASSERT(tuple_type != nullptr);
    return (tuple_type->count + 1) >= tuple_type->capacity;
}

static void tuple_type_grow(TupleType *tuple_type) {
    EXP_ASSERT(tuple_type != nullptr);
    Growth32 g        = array_growth_u32(tuple_type->capacity, sizeof(Type *));
    tuple_type->types = reallocate(tuple_type->types, g.alloc_size);
    tuple_type->capacity = g.new_capacity;
}

void tuple_type_append(TupleType *tuple_type, Type const *type) {
    EXP_ASSERT(tuple_type != nullptr);
    EXP_ASSERT(type != nullptr);
    if (tuple_type_full(tuple_type)) { tuple_type_grow(tuple_type); }
    tuple_type->types[tuple_type->count] = type;
    tuple_type->count += 1;
}

bool function_type_equality(FunctionType const *A, FunctionType const *B) {
    EXP_ASSERT(A != nullptr);
    EXP_ASSERT(B != nullptr);
    if (A == B) { return true; }
    if (!type_equal(A->return_type, B->return_type)) { return false; }
    return tuple_type_equality(&A->argument_types, &B->argument_types);
}

Type const *tuple_type_at(TupleType const *tuple_type, Scalar index) {
    EXP_ASSERT(tuple_type != nullptr);
    EXP_ASSERT(scalar_is_index(index));
    u64 i = scalar_index(index);
    EXP_ASSERT(i < tuple_type->count);
    return tuple_type->types[i];
}

Type *type_nil() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_NIL;
    return type;
}

Type *type_bool() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_BOOL;
    return type;
}

Type *type_i8() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_I8;
    return type;
}

Type *type_i16() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_I16;
    return type;
}

Type *type_i32() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_I32;
    return type;
}

Type *type_i64() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_I64;
    return type;
}

Type *type_u8() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_U8;
    return type;
}

Type *type_u16() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_U16;
    return type;
}

Type *type_u32() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_U32;
    return type;
}

Type *type_u64() {
    Type *type = callocate(1, sizeof(Type));
    type->kind = TYPE_U64;
    return type;
}

Type *type_tuple(TupleType tuple_type) {
    Type *type       = callocate(1, sizeof(Type));
    type->kind       = TYPE_TUPLE;
    type->tuple_type = tuple_type;
    return type;
}

Type *type_function(Type const *result, TupleType args) {
    EXP_ASSERT(result != nullptr);
    Type *type          = callocate(1, sizeof(Type));
    type->kind          = TYPE_FUNCTION;
    type->function_type = (FunctionType){result, args};
    return type;
}

void type_terminate(Type *type) {
    EXP_ASSERT(type != nullptr);
    switch (type->kind) {
    case TYPE_TUPLE: {
        tuple_type_terminate(&type->tuple_type);
        break;
    }

    case TYPE_FUNCTION: {
        tuple_type_terminate(&type->function_type.argument_types);
        break;
    }

    // #NOTE: no other types dynamically allocate
    default: break;
    }

    deallocate(type);
}

bool type_equal(Type const *A, Type const *B) {
    EXP_ASSERT(A != nullptr);
    EXP_ASSERT(B != nullptr);
    if (A->kind != B->kind) { return false; }

    switch (A->kind) {
    case TYPE_TUPLE: return tuple_type_equality(&A->tuple_type, &B->tuple_type);
    case TYPE_FUNCTION:
        return function_type_equality(&A->function_type, &B->function_type);

    // #NOTE: scalar types are equal when their kinds are equal
    default: return true;
    }
}

bool type_is_scalar(Type const *T) {
    EXP_ASSERT(T != nullptr);
    switch (T->kind) {
    case TYPE_NIL:
    case TYPE_BOOL:
    case TYPE_I8:
    case TYPE_I16:
    case TYPE_I32:
    case TYPE_I64:
    case TYPE_U8:
    case TYPE_U16:
    case TYPE_U32:
    case TYPE_U64:  return true;

    // #NOTE: a tuple type of size two or more cannot be scalar
    // unless we optimize it to be so. which is a TODO.
    default: return false;
    }
}

bool type_is_index(Type const *T) {
    EXP_ASSERT(T != nullptr);
    switch (T->kind) {
    case TYPE_I8:
    case TYPE_I16:
    case TYPE_I32:
    case TYPE_I64:
    case TYPE_U8:
    case TYPE_U16:
    case TYPE_U32:
    case TYPE_U64: return true;

    default: return false;
    }
}

bool type_is_arithmetic(Type const *T) {
    EXP_ASSERT(T != nullptr);
    switch (T->kind) {
    case TYPE_I8:
    case TYPE_I16:
    case TYPE_I32:
    case TYPE_I64:
    case TYPE_U8:
    case TYPE_U16:
    case TYPE_U32:
    case TYPE_U64: return true;

    default: return false;
    }
}

bool type_is_signed(Type const *T) {
    EXP_ASSERT(T != nullptr);
    switch (T->kind) {
    case TYPE_I8:
    case TYPE_I16:
    case TYPE_I32:
    case TYPE_I64: return true;

    default: return false;
    }
}

static void print_tuple_type(String *buffer, TupleType const *tuple_type) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(tuple_type != nullptr);
    string_append(buffer, SV("("));
    for (u32 i = 0; i < tuple_type->count; ++i) {
        print_type(buffer, tuple_type->types[i]);

        if (i < (tuple_type->count - 1)) { string_append(buffer, SV(", ")); }
    }
    string_append(buffer, SV(")"));
}

static void print_function_type(String *buffer,
                                FunctionType const *function_type) {
    EXP_ASSERT(function_type != nullptr);
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("fn "));
    TupleType const *tuple_type = &function_type->argument_types;
    print_tuple_type(buffer, tuple_type);
    string_append(buffer, SV(" -> "));
    print_type(buffer, function_type->return_type);
}

void print_type(String *buffer, Type const *T) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(T != nullptr);
    switch (T->kind) {
    case TYPE_NIL:      string_append(buffer, SV("nil")); break;
    case TYPE_BOOL:     string_append(buffer, SV("bool")); break;
    case TYPE_I8:       string_append(buffer, SV("i8")); break;
    case TYPE_I16:      string_append(buffer, SV("i16")); break;
    case TYPE_I32:      string_append(buffer, SV("i32")); break;
    case TYPE_I64:      string_append(buffer, SV("i64")); break;
    case TYPE_U8:       string_append(buffer, SV("u8")); break;
    case TYPE_U16:      string_append(buffer, SV("u16")); break;
    case TYPE_U32:      string_append(buffer, SV("u32")); break;
    case TYPE_U64:      string_append(buffer, SV("u64")); break;
    case TYPE_TUPLE:    print_tuple_type(buffer, &T->tuple_type); break;
    case TYPE_FUNCTION: print_function_type(buffer, &T->function_type); break;

    default: EXP_UNREACHABLE();
    }
}

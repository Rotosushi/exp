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

#include "imr/type.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/unreachable.h"

void tuple_type_initialize(TupleType *tuple_type) {
    assert(tuple_type != nullptr);
    tuple_type->capacity = 0;
    tuple_type->count    = 0;
    tuple_type->types    = NULL;
}

void tuple_type_terminate(TupleType *tuple_type) {
    assert(tuple_type != NULL);
    tuple_type->capacity = 0;
    tuple_type->count    = 0;
    deallocate(tuple_type->types);
    tuple_type->types = NULL;
}

bool tuple_type_equality(TupleType const *A, TupleType const *B) {
    assert(A != NULL);
    assert(B != NULL);
    if (A == B) { return true; }

    if (A->count != B->count) { return false; }

    for (u64 i = 0; i < A->count; ++i) {
        Type const *t = A->types[i];
        Type const *u = B->types[i];

        if (!type_equality(t, u)) { return false; }
    }

    return true;
}

static bool tuple_type_full(TupleType *tuple_type) {
    assert(tuple_type != nullptr);
    return (tuple_type->count + 1) >= tuple_type->capacity;
}

static void tuple_type_grow(TupleType *tuple_type) {
    assert(tuple_type != nullptr);
    Growth64 g        = array_growth_u64(tuple_type->capacity, sizeof(Type *));
    tuple_type->types = reallocate(tuple_type->types, g.alloc_size);
    tuple_type->capacity = g.new_capacity;
}

void tuple_type_append(TupleType *tuple_type, Type const *type) {
    assert(tuple_type != NULL);
    assert(type != nullptr);
    if (tuple_type_full(tuple_type)) { tuple_type_grow(tuple_type); }
    tuple_type->types[tuple_type->count] = type;
    tuple_type->count += 1;
}

bool function_type_equality(FunctionType const *A, FunctionType const *B) {
    assert(A != NULL);
    assert(B != NULL);
    if (A == B) { return true; }
    if (!type_equality(A->return_type, B->return_type)) { return false; }
    return tuple_type_equality(&A->argument_types, &B->argument_types);
}

Type *type_nil() {
    Type *type        = allocate(sizeof(Type));
    type->kind        = TYPE_KIND_NIL;
    type->scalar_type = 0;
    return type;
}

Type *type_boolean() {
    Type *type        = allocate(sizeof(Type));
    type->kind        = TYPE_KIND_BOOLEAN;
    type->scalar_type = 0;
    return type;
}

Type *type_i32() {
    Type *type        = allocate(sizeof(Type));
    type->kind        = TYPE_KIND_I32;
    type->scalar_type = 0;
    return type;
}

Type *type_tuple(TupleType tuple_type) {
    Type *type       = allocate(sizeof(Type));
    type->kind       = TYPE_KIND_TUPLE;
    type->tuple_type = tuple_type;
    return type;
}

Type *type_function(Type const *result, TupleType args) {
    Type *type          = allocate(sizeof(Type));
    type->kind          = TYPE_KIND_FUNCTION;
    type->function_type = (FunctionType){result, args};
    return type;
}

void type_terminate(Type *type) {
    switch (type->kind) {
    case TYPE_KIND_TUPLE: {
        tuple_type_terminate(&type->tuple_type);
        break;
    }

    case TYPE_KIND_FUNCTION: {
        tuple_type_terminate(&type->function_type.argument_types);
        break;
    }

    // #NOTE: no other types dynamically allocate
    default: break;
    }

    deallocate(type);
}

bool type_equality(Type const *A, Type const *B) {
    if (A->kind != B->kind) { return false; }

    switch (A->kind) {
    case TYPE_KIND_TUPLE:
        return tuple_type_equality(&A->tuple_type, &B->tuple_type);
    case TYPE_KIND_FUNCTION:
        return function_type_equality(&A->function_type, &B->function_type);

    // #NOTE: scalar types are equal when their kinds are equal
    default: return true;
    }
}

bool type_is_scalar(Type const *T) {
    switch (T->kind) {
    case TYPE_KIND_NIL:
    case TYPE_KIND_BOOLEAN:
    case TYPE_KIND_I32:     return true;

    // a tuple type of size two or more cannot be scalar
    // unless we optimize it to be so. which is a TODO.
    // and a tuple type of length 0 or 1 is never created.
    case TYPE_KIND_TUPLE:
    default:              return false;
    }
}

static void emit_tuple_type(TupleType const *tuple_type, String *buf) {
    string_append(buf, SV("("));
    for (u64 i = 0; i < tuple_type->count; ++i) {
        emit_type(tuple_type->types[i], buf);

        if (i < (tuple_type->count - 1)) { string_append(buf, SV(", ")); }
    }
    string_append(buf, SV(")"));
}

static void emit_function_type(FunctionType const *function_type, String *buf) {
    string_append(buf, SV("fn "));
    TupleType const *tuple_type = &function_type->argument_types;
    emit_tuple_type(tuple_type, buf);
    string_append(buf, SV(" -> "));
    emit_type(function_type->return_type, buf);
}

void emit_type(Type const *T, String *buf) {
    switch (T->kind) {
    case TYPE_KIND_NIL:      string_append(buf, SV("nil")); break;
    case TYPE_KIND_BOOLEAN:  string_append(buf, SV("bool")); break;
    case TYPE_KIND_I32:      string_append(buf, SV("i64")); break;
    case TYPE_KIND_TUPLE:    emit_tuple_type(&T->tuple_type, buf); break;
    case TYPE_KIND_FUNCTION: emit_function_type(&T->function_type, buf); break;

    default: EXP_UNREACHABLE();
    }
}

void print_type(Type const *T, FILE *file) {
    String buf;
    string_initialize(&buf);
    emit_type(T, &buf);
    print_string_view(string_to_view(&buf), file);
    string_destroy(&buf);
}

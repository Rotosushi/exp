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
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/io.h"
#include "support/unreachable.h"

TupleType tuple_type_create() {
    TupleType tuple_type;
    tuple_type.capacity = 0;
    tuple_type.size     = 0;
    tuple_type.types    = NULL;
    return tuple_type;
}

void tuple_type_destroy(TupleType *restrict tuple_type) {
    assert(tuple_type != NULL);
    tuple_type->capacity = 0;
    tuple_type->size     = 0;
    deallocate(tuple_type->types);
    tuple_type->types = NULL;
}

bool tuple_type_equality(TupleType const *A, TupleType const *B) {
    assert(A != NULL);
    assert(B != NULL);
    if (A == B) { return 1; }

    if (A->size != B->size) { return 0; }

    for (u64 i = 0; i < A->size; ++i) {
        Type *t = A->types[i];
        Type *u = B->types[i];

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

void tuple_type_append(TupleType *restrict tuple_type, Type *type) {
    assert(tuple_type != NULL);

    if (tuple_type_full(tuple_type)) { tuple_type_grow(tuple_type); }

    tuple_type->types[tuple_type->size] = type;
    tuple_type->size += 1;
}

bool function_type_equality(FunctionType const *A, FunctionType const *B) {
    assert(A != NULL);
    assert(B != NULL);
    if (A == B) { return 1; }

    if (!type_equality(A->return_type, B->return_type)) { return 0; }

    return tuple_type_equality(&A->argument_types, &B->argument_types);
}

Type type_create_nil() {
    Type type = {.kind = TYPE_KIND_NIL, .nil_type.empty = 0};
    return type;
}

Type type_create_boolean() {
    Type type = {.kind = TYPE_KIND_BOOLEAN, .boolean_type.empty = 0};
    return type;
}

Type type_create_integer() {
    Type type = {.kind = TYPE_KIND_I64, .integer_type.empty = 0};
    return type;
}

Type type_create_tuple(TupleType tuple_type) {
    Type type = {.kind = TYPE_KIND_TUPLE, .tuple_type = tuple_type};
    return type;
}

Type type_create_function(Type *result, TupleType args) {
    Type type = {
        .kind          = TYPE_KIND_FUNCTION,
        .function_type = (FunctionType){result, args}
    };
    return type;
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
    case TYPE_KIND_I64:     return true;

    // a tuple type of size two or more cannot be scalar
    // unless we optimize it to be so. which is a TODO.
    // and a tuple type of length 0 or 1 is never created.
    case TYPE_KIND_TUPLE:
    default:              return false;
    }
}

static void emit_tuple_type(TupleType const *restrict tuple_type,
                            String *restrict buf) {
    string_append(buf, SV("("));
    for (u64 i = 0; i < tuple_type->size; ++i) {
        emit_type(tuple_type->types[i], buf);

        if (i < (tuple_type->size - 1)) { string_append(buf, SV(", ")); }
    }
    string_append(buf, SV(")"));
}

static void emit_function_type(FunctionType const *restrict function_type,
                               String *restrict buf) {
    string_append(buf, SV("fn "));
    TupleType const *tuple_type = &function_type->argument_types;
    emit_tuple_type(tuple_type, buf);
    string_append(buf, SV(" -> "));
    emit_type(function_type->return_type, buf);
}

void emit_type(Type const *restrict T, String *restrict buf) {
    switch (T->kind) {
    case TYPE_KIND_NIL:      string_append(buf, SV("nil")); break;
    case TYPE_KIND_BOOLEAN:  string_append(buf, SV("bool")); break;
    case TYPE_KIND_I64:      string_append(buf, SV("i64")); break;
    case TYPE_KIND_TUPLE:    emit_tuple_type(&T->tuple_type, buf); break;
    case TYPE_KIND_FUNCTION: emit_function_type(&T->function_type, buf); break;

    default: EXP_UNREACHABLE();
    }
}

void print_type(Type const *restrict T, FILE *restrict file) {
    String buf = string_create();
    emit_type(T, &buf);
    file_write(string_to_view(&buf), file);
    string_destroy(&buf);
}

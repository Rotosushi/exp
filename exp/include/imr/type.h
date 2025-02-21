// Copyright (C) 2025 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file imr/type.h
 */

#ifndef EXP_IMR_TYPE_H
#define EXP_IMR_TYPE_H

#include "imr/scalar.h"
#include "utility/string.h"

typedef enum TypeKind {
    TYPE_NIL,
    TYPE_BOOL,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_TUPLE,
    TYPE_FUNCTION,
} TypeKind;

struct Type;

typedef struct TupleType {
    u32 count;
    u32 capacity;
    struct Type const **types;
} TupleType;

void tuple_type_initialize(TupleType *tuple_type);
void tuple_type_terminate(TupleType *tuple_type);
bool tuple_type_equality(TupleType const *A, TupleType const *B);
void tuple_type_append(TupleType *tuple_type, struct Type const *type);
struct Type const *tuple_type_at(TupleType const *tuple_type, Scalar index);

typedef struct FunctionType {
    struct Type const *return_type;
    TupleType argument_types;
} FunctionType;

bool function_type_equality(FunctionType const *A, FunctionType const *B);

/**
 * @brief represents Types in the compiler
 *
 * #TODO #FEATURE: type attributes, something like u16::max, could work in the
 * same way that struct members work, there is also no reason we cannot bind a
 * function ptr there as well, giving type "member" functions. except not tied
 * to a specific instance of that type. I think we can leverage such a mechanism
 * for type introspection if we implicitly fill in the member details when we
 * create the type.
 */
typedef struct Type {
    TypeKind kind;
    union {
        TupleType tuple_type;
        FunctionType function_type;
    };
} Type;

Type *type_nil();
Type *type_bool();
Type *type_i8();
Type *type_i16();
Type *type_i32();
Type *type_i64();
Type *type_u8();
Type *type_u16();
Type *type_u32();
Type *type_u64();
Type *type_tuple(TupleType tuple_type);
Type *type_function(Type const *result, TupleType args);
void type_terminate(Type *type);

bool type_equal(Type const *T, Type const *U);
/**
 * @brief returns true if the type is a scalar type and false otherwise.
 *
 * @note Scalar is shorthand for a type that fits in an abstract register.
 * (it just so happens to coincide with the size of a physical register on a 64
 * bit system)
 */
bool type_is_scalar(Type const *T);

/**
 * @brief returns true if the type is valid in arithmetic expressions and false
 * otherwise
 *
 * @note +, -, *, /, %
 */
bool type_is_arithmetic(Type const *T);

/**
 * @brief returns true if the type is a signed arithmetic type and false
 * otherwise.
 */
bool type_is_signed(Type const *T);

/**
 * @brief returns true if the type is valid as an index into
 * a tuple and false otherwise.
 */
bool type_is_index(Type const *T);

void print_type(String *buffer, Type const *t);

#endif // !EXP_IMR_TYPE_H

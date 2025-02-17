// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_TYPE_H
#define EXP_IMR_TYPE_H

#include "imr/scalar.h"
#include "utility/string.h"

typedef enum TypeKind {
    TYPE_KIND_NIL,
    TYPE_KIND_BOOLEAN,
    TYPE_KIND_I8,
    TYPE_KIND_I16,
    TYPE_KIND_I32,
    TYPE_KIND_I64,
    TYPE_KIND_U8,
    TYPE_KIND_U16,
    TYPE_KIND_U32,
    TYPE_KIND_U64,
    TYPE_KIND_TUPLE,
    TYPE_KIND_FUNCTION,
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

// Copyright (C) 2024 Cade Weinberg
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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_ENV_TYPE_INTERNER_H
#define EXP_ENV_TYPE_INTERNER_H
#include <stddef.h>

#include "imr/type.h"

typedef struct FunctionTypes {
    u64   size;
    u64   capacity;
    Type *types;
} FunctionTypes;

typedef struct TupleTypes {
    u64   size;
    u64   capacity;
    Type *types;
} TupleTypes;

/**
 * @brief The TypeInterner holds unique instances of
 * every type used.
 *
 * @note Types retrieved from the same TypeInterner
 * can be equality compared using their pointer values.
 *
 */
typedef struct TypeInterner {
    Type          nil_type;
    Type          boolean_type;
    Type          u8_type;
    Type          u16_type;
    Type          u32_type;
    Type          u64_type;
    Type          i8_type;
    Type          i16_type;
    Type          i32_type;
    Type          i64_type;
    TupleTypes    tuple_types;
    FunctionTypes function_types;
} TypeInterner;

/**
 * @brief create a TypeInterner
 *
 * @return TypeInterner
 */
TypeInterner type_interner_create();

/**
 * @brief destroy a TypeInterner
 *
 * @param type_interner
 */
void type_interner_destroy(TypeInterner *restrict type_interner);

Type const *type_interner_nil_type(TypeInterner *restrict type_interner);
Type const *type_interner_boolean_type(TypeInterner *restrict type_interner);
Type const *type_interner_u8_type(TypeInterner *restrict type_interner);
Type const *type_interner_u16_type(TypeInterner *restrict type_interner);
Type const *type_interner_u32_type(TypeInterner *restrict type_interner);
Type const *type_interner_u64_type(TypeInterner *restrict type_interner);
Type const *type_interner_i8_type(TypeInterner *restrict type_interner);
Type const *type_interner_i16_type(TypeInterner *restrict type_interner);
Type const *type_interner_i32_type(TypeInterner *restrict type_interner);
Type const *type_interner_i64_type(TypeInterner *restrict type_interner);
Type const *type_interner_tuple_type(TypeInterner *restrict type_interer,
                                     TupleType tuple);

Type const *type_interner_function_type(TypeInterner *restrict type_interner,
                                        Type const *return_type,
                                        TupleType   argument_types);

#endif // !EXP_ENV_TYPE_INTERNER_H

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

typedef struct TypeList {
    u64 count;
    u64 capacity;
    Type **buffer;
} TypeList;

/**
 * @brief The TypeInterner holds unique instances of
 * every type used.
 *
 * @note Types retrieved from the same TypeInterner
 * can be equality compared using their pointer values.
 *
 */
typedef struct TypeInterner {
    Type *nil_type;
    Type *boolean_type;
    Type *i64_type;
    TypeList tuple_types;
    TypeList function_types;
} TypeInterner;

void type_interner_initialize(TypeInterner *type_interner);

void type_interner_destroy(TypeInterner *type_interner);
// Type const *type_interner_nil_type(TypeInterner *type_interner);
// Type const *type_interner_boolean_type(TypeInterner *type_interner);
Type const *type_interner_i64_type(TypeInterner *type_interner);
Type const *type_interner_tuple_type(TypeInterner *type_interer,
                                     TupleType tuple);
Type const *type_interner_function_type(TypeInterner *type_interner,
                                        Type const *return_type,
                                        TupleType argument_types);

#endif // !EXP_ENV_TYPE_INTERNER_H

// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ENV_TYPE_INTERNER_H
#define EXP_ENV_TYPE_INTERNER_H
// #include <stddef.h>

#include "imr/type.h"

typedef struct TypeList {
    u32 count;
    u32 capacity;
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
    // Type *nil_type;
    // Type *boolean_type;
    Type *i32_type;
    TypeList tuple_types;
    TypeList function_types;
} TypeInterner;

void type_interner_initialize(TypeInterner *type_interner);

void type_interner_destroy(TypeInterner *type_interner);
// Type const *type_interner_nil_type(TypeInterner *type_interner);
// Type const *type_interner_boolean_type(TypeInterner *type_interner);
Type const *type_interner_i32_type(TypeInterner *type_interner);
Type const *type_interner_tuple_type(TypeInterner *type_interer,
                                     TupleType tuple);
Type const *type_interner_function_type(TypeInterner *type_interner,
                                        Type const *return_type,
                                        TupleType argument_types);

#endif // !EXP_ENV_TYPE_INTERNER_H

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
#ifndef EXP_IMR_TYPE_COMPOSITE_H
#define EXP_IMR_TYPE_COMPOSITE_H

#include "imr/type/function.h"
#include "imr/type/tuple.h"

typedef enum TypeCompositeKind {
    TYPE_COMPOSITE_KIND_TUPLE,
    TYPE_COMPOSITE_KIND_FUNCTION,
} TypeCompositeKind;

typedef union TypeCompositeData {
    TypeTuple    tuple;
    TypeFunction function;
} TypeCompositeData;

typedef struct TypeComposite {
    TypeCompositeKind kind;
    TypeCompositeData data;
} TypeComposite;

void type_composite_destroy(TypeComposite *restrict composite);

inline TypeComposite type_composite_tuple(TypeTuple tuple) {
    return (TypeComposite){TYPE_COMPOSITE_KIND_TUPLE, {tuple}};
}

inline TypeComposite
type_composite_function(struct Type const *restrict result_type,
                        TypeTuple arguments) {
    TypeComposite result = {
        TYPE_COMPOSITE_KIND_FUNCTION,
        .data.function = {result_type, arguments}
    };
    return result;
}

bool type_composite_equality(TypeComposite const *restrict A,
                             TypeComposite const *restrict B);

bool type_composite_is_callable(TypeComposite const *restrict composite);
bool type_composite_is_indexable(TypeComposite const *restrict composite);

void print_type_composite(String *restrict buffer,
                          TypeComposite const *restrict composite);

#endif // !EXP_IMR_TYPE_COMPOSITE_H

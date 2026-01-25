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

#include "imr/type/composite.h"
#include "support/assert.h"
#include "support/unreachable.h"

void type_composite_destroy(TypeComposite *restrict composite) {
    switch (composite->kind) {
    case TYPE_COMPOSITE_KIND_TUPLE:
        type_tuple_destroy(&composite->data.tuple);
        break;

    case TYPE_COMPOSITE_KIND_FUNCTION:
        type_function_destroy(&composite->data.function);
        break;

    default: EXP_UNREACHABLE();
    }
}

extern TypeComposite type_composite_tuple(TypeTuple tuple);
extern TypeComposite type_composite_function(struct Type const *argument,
                                             struct Type const *result);

bool type_composite_equality(TypeComposite const *restrict A,
                             TypeComposite const *restrict B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
    if (A->kind != B->kind) { return false; }

    switch (A->kind) {
    case TYPE_COMPOSITE_KIND_TUPLE:
        return type_tuple_equal(&A->data.tuple, &B->data.tuple);
    case TYPE_COMPOSITE_KIND_FUNCTION:
        return type_function_equal(&A->data.function, &B->data.function);

    default: EXP_UNREACHABLE();
    }
}

bool type_composite_is_callable(TypeComposite const *restrict composite) {
    exp_assert(composite != NULL);
    return composite->kind == TYPE_COMPOSITE_KIND_FUNCTION;
}

bool type_composite_is_indexable(TypeComposite const *restrict composite) {
    exp_assert(composite != NULL);
    return composite->kind == TYPE_COMPOSITE_KIND_TUPLE;
}

void print_type_composite(String *restrict buffer,
                          TypeComposite const *restrict composite) {
    switch (composite->kind) {
    case TYPE_COMPOSITE_KIND_TUPLE:
        print_type_tuple(buffer, &composite->data.tuple);
        break;

    case TYPE_COMPOSITE_KIND_FUNCTION:
        print_type_function(buffer, &composite->data.function);
        break;

    default: EXP_UNREACHABLE();
    }
}

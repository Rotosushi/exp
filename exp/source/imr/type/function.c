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

#include "imr/type/function.h"
#include "imr/type.h"
#include "support/assert.h"

void type_function_create(TypeFunction *restrict function,
                          Type const *argument,
                          Type const *result) {
    exp_assert(function != NULL);
    function->argument = argument;
    function->result   = result;
}

void type_function_destroy(TypeFunction *restrict function) {
    exp_assert(function != NULL);
    function->argument = NULL;
    function->result   = NULL;
}

bool type_function_equal(TypeFunction const *A, TypeFunction const *B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
    if (A == B) { return 1; }

    if (!type_equality(A->result, B->result)) { return 0; }
    return type_equality(A->argument, B->argument);
}

void print_type_function(String *restrict string,
                         TypeFunction const *restrict function_type) {
    string_append(string, SV("fn "));
    print_type(string, function_type->argument);
    string_append(string, SV(" -> "));
    print_type(string, function_type->result);
}

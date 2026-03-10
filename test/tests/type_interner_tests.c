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
#include <stdlib.h>

#include "test_context.h"

i32 type_interner_tests([[maybe_unused]] i32   argc,
                        [[maybe_unused]] char *argv[]) {
    Context context;
    test_context_create(&context);

    bool failure = 0;

    Type const *t0 = context_type_i64(&context);
    Type const *t1 = context_type_i64(&context);
    failure |= !type_equality(t0, t1);
    failure |= t0 != t1;

    Type const *t2 = context_type_nil(&context);
    failure |= type_equality(t2, t0);
    failure |= t0 == t2;

    context_destroy(&context);
    if (failure) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}

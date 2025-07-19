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
#include <time.h>

#include "support/random.h"
#include "test_context.h"

i32 test_constant_i64(Context *restrict context, i64 i64_) {
    Value const *value = context_constant_i64(context, i64_);

    return ((value->kind == VALUE_KIND_I64) && (value->i64_ == i64_)) ? 0 : 1;
}

i32 constants_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char *argv[]) {
    XorShiftR128PlusState rng;
    xorshiftr128plus_seed(&rng, (u64)time(NULL));
    Context context;
    test_context_create(&context);
    i32 result = 0;

    // #TODO:
    result += test_constant_i64(&context, (i64)xorshiftr128plus_next(&rng));
    result += test_constant_i64(&context, (i64)xorshiftr128plus_next(&rng));
    result += test_constant_i64(&context, (i64)xorshiftr128plus_next(&rng));
    result += test_constant_i64(&context, (i64)xorshiftr128plus_next(&rng));

    context_destroy(&context);
    return result;
}

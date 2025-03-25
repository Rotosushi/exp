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
 
#include "exp_byte.h"
#include "support/random.h"

static i32 compare_bytes(u8 const *restrict src, u8 const *restrict dst, u64 length) {
    for (u64 i = 0; i < length; i++) {
        if (dst[i] != src[i]) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static i32 test_exp_byte_copy(u8 const *restrict src, u64 length) {
    u8 dst[length];
    _exp_byte_copy(dst, src, length);
    return compare_bytes(src, dst, length);
}

static i32 test_exp_byte_copy_word(u8 const *restrict src, u64 length) {
    u8 dst[length];
    _exp_byte_copy_word(dst, src, length);
    return compare_bytes(src, dst, length);
}

struct bytes_data {
    u8 *src;
    u64 length;
};

static struct bytes_data generate_data() {
    XorShiftR128PlusState state = {0};
    xorshiftr128plus_seed(&state, (u64)random());

    struct bytes_data data;
    data.length = xorshiftr128plus_next(&state) % 4096;
    data.src = malloc(data.length * sizeof(u8));
    for (u64 i = 0; i < data.length; i++) {
        data.src[i] = (u8)(xorshiftr128plus_next(&state) % u8_MAX);
    }

    return data;
}

i32 exp_byte_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char **argv) {
    struct bytes_data data = generate_data();


    if (test_exp_byte_copy(data.src, data.length) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (test_exp_byte_copy_word(data.src, data.length) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}



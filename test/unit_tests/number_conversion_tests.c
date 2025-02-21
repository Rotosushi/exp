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

/**
 * @file unit_tests/number_conversion_tests.c
 */

#include <stdio.h>
// #include <stdlib.h>
#include <time.h>

#include "utility/numeric_conversions.h"
#include "utility/panic.h"
#include "utility/pseudo_random.h"

// static bool test_base16_uintmax_to_str(u64 value) {
//   u64 buffer_length = u64_safe_strlen(value);
//   char buffer[buffer_length + 1];
//   u64_to_str(value, buffer);

//   u64 number;
//   i32 result = sscanf(buffer, "%jx", &number);
//   if (result == EOF) { PANIC_ERRNO("sscanf failed"); }

//   return value != number;
// }

// static bool test_base8_uintmax_to_str(u64 value) {
//   u64 buffer_length = u64_safe_strlen(value);
//   char buffer[buffer_length + 1];
//   u64_to_str(value, buffer);

//   u64 number;
//   i32 result = sscanf(buffer, "%jo", &number);
//   if (result == EOF) { PANIC_ERRNO("sscanf failed"); }

//   return value != number;
// }

static bool test_base10_uintmax_to_str(u64 value) {
    u64 buffer_length = u64_safe_strlen(value);
    char buffer[buffer_length + 1];
    u64_to_str(value, buffer);

    u64 number;
    i32 result = sscanf(buffer, "%ju", &number);
    if (result == EOF) { PANIC_ERRNO("sscanf failed"); }

    return value != number;
}

static bool test_base10_intmax_to_str(i64 value) {
    u64 buffer_length = i64_safe_strlen(value);
    char buffer[buffer_length + 1];
    i64_to_str(value, buffer);

    i64 number;
    i32 result = sscanf(buffer, "%jd", &number);
    if (result == EOF) { PANIC_ERRNO("sscanf failed"); }

    return value != number;
}

i32 number_conversion_tests([[maybe_unused]] i32 argc,
                            [[maybe_unused]] char *argv[]) {
    bool failed = 0;
    XorShiftR128PlusState xorshiftr128plus_state;
    xorshiftr128plus_initialize(&xorshiftr128plus_state, (u64)time(nullptr));

    failed |= test_base10_uintmax_to_str(u64_MAX);
    failed |= test_base10_uintmax_to_str(
        xorshiftr128plus_generate(&xorshiftr128plus_state));
    failed |= test_base10_uintmax_to_str(0);
    failed |= test_base10_uintmax_to_str(1);

    // failed |= test_base16_uintmax_to_str(UINTMAX_MAX);
    // failed |= test_base16_uintmax_to_str((u64)rand());
    // failed |= test_base16_uintmax_to_str(0);
    // failed |= test_base16_uintmax_to_str(1);

    // failed |= test_base8_uintmax_to_str(UINTMAX_MAX);
    // failed |= test_base8_uintmax_to_str((u64)rand());
    // failed |= test_base8_uintmax_to_str(0);
    // failed |= test_base8_uintmax_to_str(1);

    failed |= test_base10_intmax_to_str(i64_MIN);
    failed |= test_base10_intmax_to_str(i64_MAX);
    failed |= test_base10_intmax_to_str(
        (i64)xorshiftr128plus_generate(&xorshiftr128plus_state));
    failed |= test_base10_intmax_to_str(-1);
    failed |= test_base10_intmax_to_str(0);
    failed |= test_base10_intmax_to_str(1);

    if (failed) {
        return 1;
    } else {
        return 0;
    }
}

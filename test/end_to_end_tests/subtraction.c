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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdlib.h>

#include "test_exp.h"
#include "utility/config.h"

int subtraction([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    int result = EXIT_SUCCESS;

    StringView source_path = SV(EXP_TEST_DIR "/subtraction.exp");

    result |= test_exp(source_path,
                       "fn main() { const x = 3; const y = 3; return x - y; }",
                       0);

    result |= test_exp(
        source_path,
        "fn main() { const x = 66100; const y = 66000; return x - y; }",
        100);

    result |=
        test_exp(source_path, "fn main() { const x = 3; return x - 2; }", 1);

    result |=
        test_exp(source_path, "fn main() { const x = 1; return 3 - x; }", 2);

    result |= test_exp(source_path, "fn main() { return 4 - 1; }", 3);

    return result;
}

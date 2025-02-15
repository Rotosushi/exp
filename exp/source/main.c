/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "core/compile.h"

i32 main(i32 argc, char const *argv[], [[maybe_unused]] char *envv[]) {
    return compile(argc, argv);
}

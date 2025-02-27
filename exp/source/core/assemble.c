/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include <stdlib.h>

#include "core/assemble.h"
#include "utility/process.h"

ExpResult assemble(Context *context) {
    StringView asm_path = context_assembly_path(context);
    StringView obj_path = context_object_path(context);

    char const *args[] = {
        "as", asm_path.ptr, "-o", obj_path.ptr, nullptr,
    };

    if (process("as", args) != EXIT_SUCCESS) return EXP_FAILURE;
    return EXP_SUCCESS;
}

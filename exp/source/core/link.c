/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include <stdlib.h>

#include "core/link.h"
#include "utility/config.h"
#include "utility/process.h"

ExpResult link(Context *context) {
    StringView obj_path = context_object_path(context);
    StringView out_path = context_output_path(context);

    // #TODO: place our runtime libraries into one of ld's standard search
    //  locations on install.
    // #TODO: figure out CPACK to create .deb files for installing/uninstalling
    //  exp from a host system. or creating SNAPs to do the same.
    char const *args[] = {
        "ld",     "-o",      out_path.ptr, ("-L" EXP_LIBEXP_RUNTIME_BINARY_DIR),
        "-lexps", "-lexprt", obj_path.ptr, nullptr,
    };

    if (process("ld", args) != EXIT_SUCCESS) return EXP_FAILURE;
    else return EXP_SUCCESS;
}

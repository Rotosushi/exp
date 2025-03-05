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

#include "core/link.h"
#include "support/config.h"
#include "support/process.h"

i32 link(Context *restrict context) {
    StringView obj_path = context_object_path(context);
    StringView out_path = context_output_path(context);

    // #TODO: place our target libraries into one of ld's standard search
    //  locations on install.
    // #TODO: figure out CPACK to create .deb files for installing/uninstalling
    //  exp from a host system. or creating SNAPs to do the same.
    char const *args[] = {
        "ld",
        "-o",
        out_path.ptr,
        ("-L" EXP_LIBEXP_RUNTIME_BINARY_DIR),
        "-lexp_runtime_start",
        "-lexp_runtime",
        obj_path.ptr,
        NULL,
    };

    return process("ld", args);
}

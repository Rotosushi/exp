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

#include "codegen/x86/target.h"
#include "codegen/x86/codegen.h"
#include "codegen/x86/env/context.h"
#include "codegen/x86/intrinsics/align_of.h"
#include "codegen/x86/intrinsics/size_of.h"

Target x86_target_info = {
    .tag                  = {              .length = sizeof("x86-64") - 1,.ptr = "x86-64"                                                                          },
    .triple               = {.length = sizeof("x86_64-Linux-GNU-ELF") - 1,
                             .ptr    = "x86_64-Linux-GNU-ELF"                             },
    .assembly_extension   = {                   .length = sizeof("s") - 1,      .ptr = "s"},
    .object_extension     = {                   .length = sizeof("o") - 1,      .ptr = "o"},
    .library_extension    = {                   .length = sizeof("a") - 1,      .ptr = "a"},
    .executable_extension = {                    .length = sizeof("") - 1,       .ptr = ""},
    .size_of              = x86_size_of,
    .align_of             = x86_align_of,
    .header               = x86_header,
    .codegen              = x86_codegen,
    .footer               = x86_footer,
    .context_allocate     = x86_context_allocate,
    .context_deallocate   = x86_context_deallocate
};

Target *x86_target = &x86_target_info;

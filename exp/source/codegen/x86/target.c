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

static void *x86_target_context_allocate() { return x86_context_allocate(); }

static void x86_target_context_deallocate(void *restrict context) {
    x86_context_deallocate(context);
}

Target x86_target_info = {
    .tag                  = {          .length = sizeof("x86-64") - 1,.ptr = "x86-64"                                                                      },
    .triple               = {.length = sizeof("x86_64-linux-gnu") - 1,
                             .ptr    = "x86_64-linux-gnu"                             },
    .assembly_extension   = {               .length = sizeof("s") - 1,      .ptr = "s"},
    .object_extension     = {               .length = sizeof("o") - 1,      .ptr = "o"},
    .library_extension    = {               .length = sizeof("a") - 1,      .ptr = "a"},
    .executable_extension = {                .length = sizeof("") - 1,       .ptr = ""},
    .size_of_primary      = x86_size_of_primary,
    .size_of_function     = x86_size_of_function,
    .align_of_primary     = x86_align_of_primary,
    .align_of_function    = x86_align_of_function,
    .context_allocate     = x86_target_context_allocate,
    .context_deallocate   = x86_target_context_deallocate,
    .compile_symbol       = x86_compile_symbol,
    .print_assembly       = x86_print_assembly,
};

Target *x86_target = &x86_target_info;

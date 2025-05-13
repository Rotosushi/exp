// Copyright (C) 2025 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @brief Defines the set of function pointers required of a
 * codegen target
 *
 * @note This is a work in progress, and is not intended
 * as the working version.
 */

#ifndef EXP_CODEGEN_TARGET_H
#define EXP_CODEGEN_TARGET_H

#include "support/string_view.h"

struct String;
struct Symbol;
struct Context;
struct Type;

typedef u64 (*size_of_fn)(struct Context *restrict context,
                          struct Type const *type);
typedef u64 (*align_of_fn)(struct Context *restrict context,
                           struct Type const *type);

// #NOTE with this signature we are forced into combining
// code generation with emission. However, this removes
// the need for a target dependent context, which should be
// less code overall.
typedef i32 (*codegen_fn)(struct String *restrict buffer,
                          struct Symbol const *restrict symbol,
                          struct Context *restrict context);
typedef i32 (*header_fn)(struct String *restrict buffer,
                         struct Context *restrict context);
typedef i32 (*footer_fn)(struct String *restrict buffer,
                         struct Context *restrict context);

typedef void *(*context_allocate_fn)();
typedef void (*context_deallocate_fn)(void *restrict context);

// #TODO: This structure needs to be broken up into more components
// for supporting target specific CPU features.
typedef struct Target {
    StringView            tag;
    StringView            triple;
    StringView            assembly_extension;
    StringView            object_extension;
    StringView            library_extension;
    StringView            executable_extension;
    size_of_fn            size_of;
    align_of_fn           align_of;
    header_fn             header;
    codegen_fn            codegen;
    footer_fn             footer;
    context_allocate_fn   context_allocate;
    context_deallocate_fn context_deallocate;
} Target;

#endif // !EXP_CODEGEN_TARGET_H

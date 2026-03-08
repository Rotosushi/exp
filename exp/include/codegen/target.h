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

#include "support/string.h"
#include "support/string_view.h"

struct String;
struct Symbol;
struct SymbolTable;
struct Context;
struct TypePrimary;

// #NOTE: we factor out the target size-of
// and align-of which handles composite types, because it's the
// same algorithm for any target. The only target dependent
// information is the size and alignment of the primary types,
// and the alignment requirements of functions.
typedef u64 (*size_of_primary_fn)(struct TypePrimary const *type);
typedef u64 (*align_of_primary_fn)(struct TypePrimary const *type);
//  #NOTE: The size and alignment of functions,  are for now,  the
//  size and alignment of function pointers. it just so happens that
//  on the x86-64 platform the alignment requirements of functions
//  in assembly, are the same as the alignment requirements of function
//  pointers in assembly. one of the design goals of this language is
//  functions as first class citizens within the language. and this
//  feels like a step in that direction.
typedef u64 (*size_of_function_fn)();
typedef u64 (*align_of_function_fn)();

/**
 * @brief allocate a target dependent context
 *
 * This context will hold all of the needed information to handle
 * the target specific in memory representation.
 */
typedef void *(*target_context_allocate_fn)();

/**
 * @brief Clean up the memory allocated to the target dependent context
 */
typedef void (*target_context_deallocate_fn)(void *);

/**
 * @brief Compile the target independent representation into the
 * target dependent representation.
 */
typedef void (*target_compile_symbol_fn)(struct Symbol *restrict symbol,
                                         struct Context *restrict context);

/**
 * @brief Print the target dependent assembly to the given string.
 *
 * Shall be called after the entire translation unit has been generated.
 *
 * @param String * the buffer the buffer to print the assembly to
 * @param Context * the translation unit to print the assembly from
 */
typedef void (*target_print_assembly_fn)(String *restrict buffer,
                                         struct Context *restrict context);

// #TODO: Extend the structure to specify CPU specific extensions.
typedef struct Target {
    StringView                   tag;
    StringView                   triple;
    StringView                   assembly_extension;
    StringView                   object_extension;
    StringView                   library_extension;
    StringView                   executable_extension;
    size_of_primary_fn           size_of_primary;
    size_of_function_fn          size_of_function;
    align_of_primary_fn          align_of_primary;
    align_of_function_fn         align_of_function;
    target_context_allocate_fn   context_allocate;
    target_context_deallocate_fn context_deallocate;
    target_compile_symbol_fn     compile_symbol;
    target_print_assembly_fn     print_assembly;
} Target;

#endif // !EXP_CODEGEN_TARGET_H

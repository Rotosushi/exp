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
#include <string.h>

#include "env/context.h"
#include "imr/value/function.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void formal_argument_list_create(FormalArgumentList *restrict fal) {
    EXP_ASSERT(fal != NULL);
    fal->capacity = 0;
    fal->length   = 0;
    fal->list     = NULL;
}

void formal_argument_list_destroy(FormalArgumentList *restrict fal) {
    EXP_ASSERT(fal != NULL);
    fal->capacity = 0;
    fal->length   = 0;
    deallocate(fal->list);
    fal->list = NULL;
}

static bool formal_argument_list_full(FormalArgumentList *restrict fal) {
    return fal->capacity <= (fal->length + 1);
}

static void formal_argument_list_grow(FormalArgumentList *restrict fal) {
    Growth_u8 g   = array_growth_u8(fal->capacity, sizeof(Local *));
    fal->list     = reallocate(fal->list, g.alloc_size);
    fal->capacity = g.new_capacity;
}

Local *formal_argument_list_append(FormalArgumentList *restrict fal,
                                   Local *restrict arg) {
    EXP_ASSERT(fal != NULL);
    EXP_ASSERT(arg != NULL);
    if (formal_argument_list_full(fal)) { formal_argument_list_grow(fal); }

    fal->list[fal->length++] = arg;

    return arg;
}

void function_create(Function *restrict function) {
    EXP_ASSERT(function != NULL);
    formal_argument_list_create(&function->arguments);
    locals_create(&function->locals);
    block_create(&function->body);
    u32 ssa          = locals_declare(&function->locals);
    function->result = locals_lookup(&function->locals, ssa);
}

void function_destroy(Function *restrict function) {
    EXP_ASSERT(function != NULL);
    formal_argument_list_destroy(&function->arguments);
    locals_destroy(&function->locals);
    block_destroy(&function->body);
    function->result = NULL;
}

u32 function_declare_argument(Function *restrict function) {
    EXP_ASSERT(function != NULL);
    u32    ssa   = locals_declare(&function->locals);
    Local *local = locals_lookup(&function->locals, ssa);
    formal_argument_list_append(&function->arguments, local);
    return ssa;
}

u32 function_declare_local(Function *restrict function) {
    EXP_ASSERT(function != NULL);
    return locals_declare(&function->locals);
}

Local *function_lookup_argument(Function const *restrict function, u8 index) {
    EXP_ASSERT(function != NULL);
    EXP_ASSERT(index < function->arguments.length);
    return function->arguments.list[index];
}

Local *function_lookup_local(Function const *restrict function, u32 ssa) {
    EXP_ASSERT(function != NULL);
    return locals_lookup(&function->locals, ssa);
}

Local *function_lookup_local_name(Function const *restrict function,
                                  StringView name) {
    EXP_ASSERT(function != NULL);
    return locals_lookup_name(&function->locals, name);
}

u32 function_locals_length(Function const *restrict function) {
    EXP_ASSERT(function != NULL);
    return function->locals.size;
}

u8 function_arguments_length(Function const *restrict function) {
    EXP_ASSERT(function != NULL);
    return function->arguments.length;
}

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
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "env/context.h"
#include "imr/function.h"
#include "support/allocation.h"
#include "support/array_growth.h"

void formal_argument_list_create(FormalArgumentList *restrict fal) {
    assert(fal != NULL);
    fal->capacity = 0;
    fal->size     = 0;
    fal->list     = NULL;
}

void formal_argument_list_destroy(FormalArgumentList *restrict fal) {
    assert(fal != NULL);
    fal->capacity = 0;
    fal->size     = 0;
    deallocate(fal->list);
    fal->list = NULL;
}

static bool formal_argument_list_full(FormalArgumentList *restrict fal) {
    return fal->capacity <= (fal->size + 1);
}

static void formal_argument_list_grow(FormalArgumentList *restrict fal) {
    Growth_u8 g   = array_growth_u8(fal->capacity, sizeof(Local *));
    fal->list     = reallocate(fal->list, g.alloc_size);
    fal->capacity = g.new_capacity;
}

Local *formal_argument_list_append(FormalArgumentList *restrict fal,
                                   Local *restrict arg) {
    assert(fal != NULL);
    assert(arg != NULL);
    if (formal_argument_list_full(fal)) { formal_argument_list_grow(fal); }

    fal->list[fal->size++] = arg;

    return arg;
}

void function_create(Function *restrict function) {
    assert(function != NULL);
    formal_argument_list_create(&function->arguments);
    locals_create(&function->locals);
    bytecode_create(&function->body);
    function->return_type = NULL;
}

void function_destroy(Function *restrict function) {
    assert(function != NULL);
    formal_argument_list_destroy(&function->arguments);
    locals_destroy(&function->locals);
    bytecode_destroy(&function->body);
    function->return_type = NULL;
}

u32 function_declare_argument(Function *restrict function) {
    assert(function != NULL);
    u32    ssa   = locals_declare(&function->locals);
    Local *local = locals_lookup(&function->locals, ssa);
    formal_argument_list_append(&function->arguments, local);
    return ssa;
}

Local *function_lookup_argument(Function const *restrict function, u8 index) {
    assert(function != NULL);
    assert(index < function->arguments.size);
    return function->arguments.list[index];
}

u32 function_declare_local(Function *restrict function) {
    assert(function != NULL);
    return locals_declare(&function->locals);
}

Local *function_lookup_local(Function const *restrict function, u32 ssa) {
    assert(function != NULL);

    return locals_lookup(&function->locals, ssa);
}

Local *function_lookup_local_name(Function const *restrict function,
                                  StringView name) {
    assert(function != NULL);

    return locals_lookup_name(&function->locals, name);
}

u32 function_locals_length(Function const *restrict function) {
    assert(function != NULL);
    return function->locals.size;
}

u8 function_arguments_length(Function const *restrict function) {
    assert(function != NULL);
    return function->arguments.size;
}

static void print_formal_argument(String *restrict string,
                                  Local *restrict arg) {
    string_append(string, arg->name);
    string_append(string, SV(": "));
    print_type(string, arg->type);
}

void print_function(String *restrict string,
                    Function const *restrict f,
                    Context *restrict context) {
    string_append(string, SV("("));
    FormalArgumentList const *args = &f->arguments;
    for (u8 i = 0; i < args->size; ++i) {
        print_formal_argument(string, args->list[i]);

        if (i < (u8)(args->size - 1)) { string_append(string, SV(", ")); }
    }
    string_append(string, SV(")\n"));
    print_bytecode(string, &f->body, context);
}

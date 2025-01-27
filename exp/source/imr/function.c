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
// #include <stdlib.h>
// #include <string.h>

#include "env/context.h"
#include "imr/function.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

static void formal_argument_list_create(FormalArgumentList *arguments) {
    assert(arguments != nullptr);
    arguments->capacity = 0;
    arguments->size     = 0;
    arguments->list     = nullptr;
}

static void formal_argument_list_destroy(FormalArgumentList *arguments) {
    assert(arguments != nullptr);
    arguments->capacity = 0;
    arguments->size     = 0;
    deallocate(arguments->list);
    arguments->list = nullptr;
}

static bool formal_argument_list_full(FormalArgumentList *arguments) {
    assert(arguments != nullptr);
    return (arguments->size + 1) >= arguments->capacity;
}

static void formal_argument_list_grow(FormalArgumentList *arguments) {
    assert(arguments != nullptr);
    Growth8 g           = array_growth_u8(arguments->capacity, sizeof(Local *));
    arguments->list     = reallocate(arguments->list, g.alloc_size);
    arguments->capacity = g.new_capacity;
}

static void formal_argument_list_append(FormalArgumentList *arguments,
                                        Local *arg) {
    assert(arguments != nullptr);

    if (formal_argument_list_full(arguments)) {
        formal_argument_list_grow(arguments);
    }

    arguments->list[arguments->size] = arg;
    arguments->size += 1;
}

static Local *formal_argument_list_at(FormalArgumentList *arguments, u8 index) {
    assert(arguments != nullptr);
    assert(index < arguments->size);
    Local *formal_argument = arguments->list[index];
    assert(formal_argument != nullptr);
    return formal_argument;
}

static Local *formal_argument_list_lookup(FormalArgumentList *arguments,
                                          StringView name) {
    assert(arguments != nullptr);
    assert(!string_view_empty(name));

    for (u8 i = 0; i < arguments->size; ++i) {
        Local *formal_argument = arguments->list[i];
        assert(formal_argument != nullptr);
        if (string_view_equality(formal_argument->label, name)) {
            return formal_argument;
        }
    }

    return nullptr;
}

void function_initialize(Function *function_body) {
    assert(function_body != nullptr);
    formal_argument_list_create(&function_body->arguments);
    local_allocator_initialize(&function_body->allocator);
    block_initialize(&function_body->block);
    function_body->return_type = nullptr;
}

void function_terminate(Function *function) {
    assert(function != nullptr);
    formal_argument_list_destroy(&function->arguments);
    local_allocator_terminate(&function->allocator);
    block_terminate(&function->block);
    function->return_type = nullptr;
}

void function_append_argument(Function *function, Local *argument) {
    assert(function != nullptr);
    assert(argument != nullptr);
    formal_argument_list_append(&function->arguments, argument);
}

Local *function_arguments_lookup(Function *function, StringView name) {
    assert(function != nullptr);
    assert(!string_view_empty(name));
    return formal_argument_list_lookup(&function->arguments, name);
}

Local *function_arguments_at(Function *function, u8 index) {
    assert(function != nullptr);
    return formal_argument_list_at(&function->arguments, index);
}

u32 function_declare_local(Function *function) {
    assert(function != nullptr);
    return local_allocator_declare_ssa(&function->allocator);
}

Local *function_local_at(Function *function, u32 ssa) {
    assert(function != nullptr);
    return local_allocator_at(&function->allocator, ssa);
}
/*
Local *function_local_at_name(Function *function, StringView name) {
    assert(function != nullptr);
    return local_allocator_at_name(&function->allocator, name);
}


void function_allocate_result(Function *function, Local *local) {
    assert(function != nullptr);
    assert(local != nullptr);
    local_allocator_allocate_result(&function->allocator, local);
}

void function_allocate_formal_argument(Function *function,
                                       Local *local,
                                       u8 argument_index) {
    assert(function != nullptr);
    assert(local != nullptr);
    local_allocator_allocate_formal_argument(
        &function->allocator, local, argument_index);
}

void function_allocate_actual_argument(Function *function,
                                       Local *local,
                                       u8 argument_index,
                                       u32 block_index) {
    assert(function != nullptr);
    assert(local != nullptr);
    local_allocator_allocate_actual_argument(
        &function->allocator, local, argument_index, block_index);
}

*/

void function_allocate_local(Function *function,
                             Local *local,
                             u32 block_index) {
    assert(function != nullptr);
    assert(local != nullptr);
    local_allocator_allocate_local(&function->allocator, local, block_index);
}

void function_append_instruction(Function *function, Instruction instruction) {
    assert(function != nullptr);
    block_append(&function->block, instruction);
}

static void print_formal_argument(String *buffer, Local *formal_argument) {
    assert(buffer != nullptr);
    assert(formal_argument != nullptr);
    assert(!string_view_empty(formal_argument->label));
    assert(formal_argument->type != nullptr);
    string_append(buffer, formal_argument->label);
    string_append(buffer, SV(": "));
    print_type(buffer, formal_argument->type);
}

void print_function(String *buffer,
                    Function const *function,
                    Context *context) {
    string_append(buffer, SV("("));
    FormalArgumentList const *args = &function->arguments;
    for (u8 index = 0; index < args->size; ++index) {
        Local *formal_argument = args->list[index];
        assert(formal_argument != nullptr);
        print_formal_argument(buffer, formal_argument);

        if (index < (u8)(args->size - 1)) { string_append(buffer, SV(", ")); }
    }
    string_append(buffer, SV(")"));

    if (function->return_type != nullptr) {
        string_append(buffer, SV(" -> "));
        print_type(buffer, function->return_type);
    }

    string_append(buffer, SV("\n"));
    print_block(buffer, &function->block, context);
}

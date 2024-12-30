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
#include "imr/function_body.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

void formal_argument_list_create(FormalArgumentList *arguments) {
    assert(arguments != nullptr);
    arguments->capacity = 0;
    arguments->size     = 0;
    arguments->list     = nullptr;
}

void formal_argument_list_destroy(FormalArgumentList *arguments) {
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
    Growth8 g = array_growth_u8(arguments->capacity, sizeof(FormalArgument));
    arguments->list     = reallocate(arguments->list, g.alloc_size);
    arguments->capacity = g.new_capacity;
}

void formal_argument_list_append(FormalArgumentList *arguments,
                                 FormalArgument arg) {
    assert(arguments != nullptr);

    if (formal_argument_list_full(arguments)) {
        formal_argument_list_grow(arguments);
    }

    arguments->list[arguments->size] = arg;
    arguments->size += 1;
}

FormalArgument *formal_argument_list_at(FormalArgumentList *arguments,
                                        u8 index) {
    assert(arguments != nullptr);
    assert(index < arguments->size);
    return arguments->list + index;
}

FormalArgument *formal_argument_list_lookup(FormalArgumentList *arguments,
                                            StringView name) {
    assert(arguments != nullptr);

    for (u8 i = 0; i < arguments->size; ++i) {
        FormalArgument *arg = arguments->list + i;
        if (string_view_equality(arg->name, name)) { return arg; }
    }

    return nullptr;
}

void function_body_initialize(FunctionBody *function_body) {
    assert(function_body != nullptr);
    formal_argument_list_create(&function_body->arguments);
    local_allocator_initialize(&function_body->allocator);
    block_initialize(&function_body->block);
    function_body->return_type = nullptr;
}

void function_body_terminate(FunctionBody *function) {
    assert(function != nullptr);
    formal_argument_list_destroy(&function->arguments);
    local_allocator_terminate(&function->allocator);
    block_terminate(&function->block);
    function->return_type = nullptr;
}

void function_body_allocate_argument(FunctionBody *function,
                                     FormalArgument argument) {
    assert(function != nullptr);
    argument.ssa = local_allocator_declare_ssa(&function->allocator);
    formal_argument_list_append(&function->arguments, argument);
    Local *local = local_allocator_at(&function->allocator, argument.ssa);
    assert(local != nullptr);
    local_update_type(local, argument.type);
    local_update_label(local, argument.name);
}

FormalArgument *function_body_arguments_lookup(FunctionBody *function,
                                               StringView name) {
    assert(function != nullptr);
    assert(!string_view_empty(name));
    return formal_argument_list_lookup(&function->arguments, name);
}

FormalArgument *function_body_arguments_at(FunctionBody *function, u8 index) {
    assert(function != nullptr);
    return formal_argument_list_at(&function->arguments, index);
}

u64 function_body_declare_local(FunctionBody *function) {
    assert(function != nullptr);
    return local_allocator_declare_ssa(&function->allocator);
}

Local *function_body_local_at(FunctionBody *function, u64 ssa) {
    assert(function != nullptr);
    return local_allocator_at(&function->allocator, ssa);
}

Local *function_body_local_at_name(FunctionBody *function, StringView name) {
    assert(function != nullptr);
    return local_allocator_at_name(&function->allocator, name);
}

void function_body_allocate_local(FunctionBody *function,
                                  Local *local,
                                  u64 block_index) {
    assert(function != nullptr);
    assert(local != nullptr);
    local_allocator_allocate_local(&function->allocator, local, block_index);
}

void function_body_append_instruction(FunctionBody *function,
                                      Instruction instruction) {
    assert(function != nullptr);
    block_append(&function->block, instruction);
}

static void print_formal_argument(FormalArgument *arg, String *buffer) {
    string_append(buffer, arg->name);
    string_append(buffer, SV(": "));
    print_type(buffer, arg->type);
}

void print_function_body(String *buffer,
                         FunctionBody const *f,
                         Context *context) {
    string_append(buffer, SV("("));
    FormalArgumentList const *args = &f->arguments;
    for (u8 i = 0; i < args->size; ++i) {
        print_formal_argument(args->list + i, buffer);

        if (i < (u8)(args->size - 1)) { string_append(buffer, SV(", ")); }
    }
    string_append(buffer, SV(")\n"));
    print_block(buffer, &f->block, context);
}

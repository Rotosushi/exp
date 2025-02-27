/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "imr/function.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"

static void formal_arguments_initialize(FormalArguments *arguments) {
    EXP_ASSERT(arguments != nullptr);
    arguments->capacity = 0;
    arguments->length   = 0;
    arguments->buffer   = nullptr;
}

static void formal_arguments_terminate(FormalArguments *arguments) {
    EXP_ASSERT(arguments != nullptr);
    deallocate(arguments->buffer);
    formal_arguments_initialize(arguments);
}

static bool formal_arguments_full(FormalArguments const *arguments) {
    EXP_ASSERT(arguments != nullptr);
    return (arguments->length + 1) >= arguments->capacity;
}

static void formal_arguments_grow(FormalArguments *arguments) {
    EXP_ASSERT(arguments != nullptr);
    Growth8 g = array_growth_u8(arguments->capacity, sizeof(FormalArgument));
    arguments->buffer   = reallocate(arguments->buffer, g.alloc_size);
    arguments->capacity = g.new_capacity;
}

static void formal_arguments_append(FormalArguments *arguments,
                                    FormalArgument arg) {
    EXP_ASSERT(arguments != nullptr);
    if (formal_arguments_full(arguments)) { formal_arguments_grow(arguments); }

    arguments->buffer[arguments->length++] = arg;
}

static FormalArgument formal_arguments_at(FormalArguments *arguments,
                                          u8 index) {
    EXP_ASSERT(arguments != nullptr);
    EXP_ASSERT(index < arguments->length);
    return arguments->buffer[index];
}

static FormalArgument *formal_argument_list_lookup(FormalArguments *arguments,
                                                   StringView name) {
    EXP_ASSERT(arguments != nullptr);
    EXP_ASSERT(!string_view_empty(name));

    for (u8 index = 0; index < arguments->length; ++index) {
        FormalArgument *formal_argument = arguments->buffer + index;
        if (string_view_equality(formal_argument->name, name)) {
            return formal_argument;
        }
    }

    return nullptr;
}

void function_initialize(Function *function) {
    EXP_ASSERT(function != nullptr);
    formal_arguments_initialize(&function->arguments);
    block_initialize(&function->block);
    function->return_type = nullptr;
}

void function_terminate(Function *function) {
    EXP_ASSERT(function != nullptr);
    formal_arguments_terminate(&function->arguments);
    block_terminate(&function->block);
    function->return_type = nullptr;
}

void function_arguments_append(Function *function, FormalArgument argument) {
    EXP_ASSERT(function != nullptr);
    formal_arguments_append(&function->arguments, argument);
}

FormalArgument *function_arguments_lookup(Function *function, StringView name) {
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(!string_view_empty(name));
    return formal_argument_list_lookup(&function->arguments, name);
}

FormalArgument function_arguments_at(Function *function, u8 index) {
    EXP_ASSERT(function != nullptr);
    return formal_arguments_at(&function->arguments, index);
}

void function_append_instruction(Function *function, Instruction instruction) {
    EXP_ASSERT(function != nullptr);
    block_append(&function->block, instruction);
}

static void print_formal_argument(String *buffer,
                                  FormalArgument const *formal_argument) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(formal_argument != nullptr);
    EXP_ASSERT(!string_view_empty(formal_argument->name));
    EXP_ASSERT(formal_argument->type != nullptr);
    string_append(buffer, formal_argument->name);
    string_append(buffer, SV(": "));
    print_type(buffer, formal_argument->type);
}

static void print_formal_arguments(String *buffer,
                                   FormalArguments const *formal_arguments) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(formal_arguments != nullptr);
    string_append(buffer, SV("("));
    for (u8 index = 0; index < formal_arguments->length; ++index) {
        FormalArgument const *formal_argument =
            formal_arguments->buffer + index;
        print_formal_argument(buffer, formal_argument);

        if (index < (u8)(formal_arguments->length - 1)) {
            string_append(buffer, SV(", "));
        }
    }
    string_append(buffer, SV(")"));
}

static void print_function_type(String *buffer,
                                FormalArguments const *arguments,
                                Type const *return_type) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(arguments != nullptr);
    EXP_ASSERT(return_type != nullptr);
    print_formal_arguments(buffer, arguments);
    if (return_type != nullptr) {
        string_append(buffer, SV(" -> "));
        print_type(buffer, return_type);
    }
}

void print_function(String *buffer, Function const *function) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(function != nullptr);
    print_function_type(buffer, &function->arguments, function->return_type);
    string_append(buffer, SV("\n"));
    print_block(buffer, &function->block);
}

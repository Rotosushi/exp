/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "imr/function.h"
#include "env/context.h"
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
    Growth8 g           = array_growth_u8(arguments->capacity, sizeof(Local *));
    arguments->buffer   = reallocate(arguments->buffer, g.alloc_size);
    arguments->capacity = g.new_capacity;
}

static void formal_arguments_append(FormalArguments *arguments, Local arg) {
    EXP_ASSERT(arguments != nullptr);
    if (formal_arguments_full(arguments)) { formal_arguments_grow(arguments); }

    arguments->buffer[arguments->length] = arg;
    arguments->length += 1;
}

static Local *formal_arguments_at(FormalArguments *arguments, u8 index) {
    EXP_ASSERT(arguments != nullptr);
    EXP_ASSERT(index < arguments->length);
    return arguments->buffer + index;
}

static Local *formal_argument_list_lookup(FormalArguments *arguments,
                                          StringView name) {
    EXP_ASSERT(arguments != nullptr);
    EXP_ASSERT(!string_view_empty(name));

    for (u8 index = 0; index < arguments->length; ++index) {
        Local *formal_argument = arguments->buffer + index;
        if (string_view_equality(formal_argument->label, name)) {
            return formal_argument;
        }
    }

    return nullptr;
}

static void locals_initialize(Locals *locals) {
    EXP_ASSERT(locals != nullptr);
    locals->length   = 0;
    locals->capacity = 0;
    locals->buffer   = nullptr;
}

static void locals_terminate(Locals *locals) {
    EXP_ASSERT(locals != nullptr);
    deallocate(locals->buffer);
    locals_initialize(locals);
}

static bool locals_full(Locals const *locals) {
    EXP_ASSERT(locals != nullptr);
    return (locals->capacity + 1) >= locals->capacity;
}

static void locals_grow(Locals *locals) {
    EXP_ASSERT(locals != nullptr);
    Growth32 g       = array_growth_u32(locals->capacity, sizeof(Local));
    locals->buffer   = reallocate(locals->buffer, g.alloc_size);
    locals->capacity = g.new_capacity;
}

static u32 locals_allocate(Locals *locals) {
    EXP_ASSERT(locals != nullptr);

    if (locals_full(locals)) { locals_grow(locals); }

    u32 index    = locals->length++;
    Local *local = locals->buffer + index;
    local_initialize(local);
    return index;
}

static Local *locals_at(Locals *locals, u32 index) {
    EXP_ASSERT(locals != nullptr);
    EXP_ASSERT(index < locals->length);
    return locals->buffer + index;
}

void function_initialize(Function *function) {
    EXP_ASSERT(function != nullptr);
    formal_arguments_initialize(&function->arguments);
    locals_initialize(&function->locals);
    block_initialize(&function->block);
    function->return_type = nullptr;
}

void function_terminate(Function *function) {
    EXP_ASSERT(function != nullptr);
    formal_arguments_terminate(&function->arguments);
    locals_terminate(&function->locals);
    block_terminate(&function->block);
    function->return_type = nullptr;
}

void function_arguments_append(Function *function, Local argument) {
    EXP_ASSERT(function != nullptr);
    formal_arguments_append(&function->arguments, argument);
}

Local *function_arguments_lookup(Function *function, StringView name) {
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(!string_view_empty(name));
    return formal_argument_list_lookup(&function->arguments, name);
}

Local *function_arguments_at(Function *function, u8 index) {
    EXP_ASSERT(function != nullptr);
    return formal_arguments_at(&function->arguments, index);
}

u32 function_declare_local(Function *function) {
    EXP_ASSERT(function != nullptr);
    return locals_allocate(&function->locals);
}

Local *function_local_at(Function *function, u32 ssa) {
    EXP_ASSERT(function != nullptr);
    return locals_at(&function->locals, ssa);
}

void function_append_instruction(Function *function, Instruction instruction) {
    EXP_ASSERT(function != nullptr);
    block_append(&function->block, instruction);
}

static void print_formal_argument(String *buffer,
                                  Local const *formal_argument) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(formal_argument != nullptr);
    EXP_ASSERT(!string_view_empty(formal_argument->label));
    EXP_ASSERT(formal_argument->type != nullptr);
    string_append(buffer, formal_argument->label);
    string_append(buffer, SV(": "));
    print_type(buffer, formal_argument->type);
}

void print_function(String *buffer, Function const *function,
                    Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);
    string_append(buffer, SV("("));
    FormalArguments const *args = &function->arguments;
    for (u8 index = 0; index < args->length; ++index) {
        Local const *formal_argument = args->buffer + index;
        print_formal_argument(buffer, formal_argument);

        if (index < (u8)(args->length - 1)) { string_append(buffer, SV(", ")); }
    }
    string_append(buffer, SV(")"));

    if (function->return_type != nullptr) {
        string_append(buffer, SV(" -> "));
        print_type(buffer, function->return_type);
    }

    string_append(buffer, SV("\n"));
    print_block(buffer, &function->block, context);
}

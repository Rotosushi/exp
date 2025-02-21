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

/**
 * @file imr/function.c
 */
#include "imr/function.h"
#include "env/context.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
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

Operand function_append_return(Function *function, Context *context,
                               Operand result) {
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);
    block_append(&function->block, instruction_return(result));
    return result;
}

Operand function_append_call(Function *function, Context *context,
                             Operand label, Operand args) {
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);
    u32 stack_index = context_stack_push(context, )
}
Operand function_append_dot(Function *function, Context *context, Operand src,
                            Operand index);
Operand function_append_load(Function *function, Context *context, Operand src);
Operand function_append_neg(Function *function, Context *context, Operand src);
Operand function_append_add(Function *function, Context *context, Operand left,
                            Operand right);
Operand function_append_sub(Function *function, Context *context, Operand left,
                            Operand right);
Operand function_append_mul(Function *function, Context *context, Operand left,
                            Operand right);
Operand function_append_div(Function *function, Context *context, Operand left,
                            Operand right);
Operand function_append_mod(Function *function, Context *context, Operand left,
                            Operand right);

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

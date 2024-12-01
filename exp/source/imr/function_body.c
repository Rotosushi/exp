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

    arg.index                        = arguments->size;
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

void local_variables_initialize(LocalVariables *locals) {
    assert(locals != nullptr);
    locals->size     = 0;
    locals->capacity = 0;
    locals->buffer   = nullptr;
}

static void local_variables_destroy(LocalVariables *locals) {
    assert(locals != nullptr);
    locals->size     = 0;
    locals->capacity = 0;
    deallocate(locals->buffer);
    locals->buffer = nullptr;
}

static bool local_variables_full(LocalVariables *locals) {
    assert(locals != nullptr);
    return (locals->size + 1) >= locals->capacity;
}

static void local_variables_grow(LocalVariables *locals) {
    assert(locals != nullptr);
    Growth64 g     = array_growth_u64(locals->capacity, sizeof(LocalVariable));
    locals->buffer = reallocate(locals->buffer, g.alloc_size);
    locals->capacity = g.new_capacity;
}

void local_variables_append(LocalVariables *locals, LocalVariable var) {
    assert(locals != nullptr);
    if (local_variables_full(locals)) { local_variables_grow(locals); }

    locals->buffer[locals->size] = var;
    locals->size += 1;
}

static void
local_variables_name_ssa(LocalVariables *locals, u16 ssa, StringView name) {
    assert(locals != nullptr);
    for (u64 i = 0; i < locals->size; ++i) {
        LocalVariable *var = locals->buffer + i;
        if (var->ssa == ssa) {
            var->name = name;
            return;
        }
    }
}

LocalVariable *local_variables_lookup(LocalVariables *locals, StringView name) {
    assert(locals != nullptr);
    for (u64 i = 0; i < locals->size; ++i) {
        LocalVariable *var = locals->buffer + i;
        if (string_view_equality(var->name, name)) { return var; }
    }
    return nullptr;
}

LocalVariable *local_variables_lookup_ssa(LocalVariables *locals, u16 ssa) {
    assert(locals != nullptr);
    for (u64 i = 0; i < locals->size; ++i) {
        LocalVariable *var = locals->buffer + i;
        if (var->ssa == ssa) { return var; }
    }
    return nullptr;
}

void function_body_initialize(FunctionBody *function_body) {
    assert(function_body != nullptr);
    formal_argument_list_create(&function_body->arguments);
    local_variables_initialize(&function_body->locals);
    block_initialize(&function_body->block);
    function_body->return_type = nullptr;
    function_body->ssa_count   = 0;
}

void function_body_terminate(FunctionBody *function) {
    assert(function != nullptr);
    formal_argument_list_destroy(&function->arguments);
    local_variables_destroy(&function->locals);
    block_terminate(&function->block);
    function->return_type = nullptr;
    function->ssa_count   = 0;
}

void function_body_new_argument(FunctionBody *function,
                                FormalArgument argument) {
    assert(function != nullptr);

    u16 ssa = (u16)(function->ssa_count++);
    assert(function->ssa_count <= u16_MAX);
    LocalVariable local_arg = {
        .name = argument.name, .type = argument.type, .ssa = ssa};
    argument.ssa = local_arg.ssa;

    local_variables_append(&function->locals, local_arg);
    formal_argument_list_append(&function->arguments, argument);
}

FormalArgument *function_body_arguments_lookup(FunctionBody *function,
                                               StringView name) {
    assert(function != nullptr);
    return formal_argument_list_lookup(&function->arguments, name);
}

FormalArgument *function_body_arguments_at(FunctionBody *function, u8 index) {
    assert(function != nullptr);
    return formal_argument_list_at(&function->arguments, index);
}

void function_body_new_local(FunctionBody *function, StringView name, u16 ssa) {
    assert(function != nullptr);
    local_variables_name_ssa(&function->locals, ssa, name);
}

Operand function_body_new_ssa(FunctionBody *function) {
    assert(function != nullptr);
    u16 ssa = (u16)(function->ssa_count++);
    assert(function->ssa_count <= u16_MAX);
    LocalVariable local = {.name = SV(""), .type = nullptr, .ssa = ssa};
    local_variables_append(&function->locals, local);
    return operand_ssa(local.ssa);
}

LocalVariable *function_body_locals_lookup(FunctionBody *function,
                                           StringView name) {
    assert(function != nullptr);
    return local_variables_lookup(&function->locals, name);
}

LocalVariable *function_body_locals_ssa(FunctionBody *function, u16 ssa) {
    assert(function != nullptr);
    return local_variables_lookup_ssa(&function->locals, ssa);
}

static void print_formal_argument(FormalArgument *arg, FILE *file) {
    file_write(arg->name.ptr, file);
    file_write(": ", file);
    print_type(arg->type, file);
}

void print_function_body(FunctionBody const *f, FILE *file, Context *context) {
    file_write("(", file);
    FormalArgumentList const *args = &f->arguments;
    for (u8 i = 0; i < args->size; ++i) {
        print_formal_argument(args->list + i, file);

        if (i < (u8)(args->size - 1)) { file_write(", ", file); }
    }
    file_write(")\n", file);
    print_block(&f->block, file, context);
}

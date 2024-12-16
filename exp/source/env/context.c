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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <stdlib.h>

#include "env/context.h"

void context_initialize(Context *context, CLIOptions *options) {
    assert(options != nullptr);
    assert(context != nullptr);
    context_options_initialize(&context->options, options);
    string_interner_initialize(&context->string_interner);
    type_interner_initialize(&context->type_interner);
    symbol_table_create(&context->symbol_table);
    labels_initialize(&context->labels);
    constants_initialize(&context->constants);
    error_initialize(&context->current_error);
    context->current_function = nullptr;
}

void context_terminate(Context *context) {
    assert(context != nullptr);
    context_options_terminate(&(context->options));
    string_interner_terminate(&(context->string_interner));
    type_interner_destroy(&(context->type_interner));
    symbol_table_destroy(&(context->symbol_table));
    labels_terminate(&(context->labels));
    constants_terminate(&(context->constants));
    error_terminate(&context->current_error);
    context->current_function = nullptr;
}

bool context_do_assemble(Context *context) {
    assert(context != nullptr);
    return context_options_do_assemble(&context->options);
}

bool context_do_link(Context *context) {
    assert(context != nullptr);
    return context_options_do_link(&context->options);
}

bool context_do_cleanup(Context *context) {
    assert(context != nullptr);
    return context_options_do_cleanup(&context->options);
}

StringView context_source_path(Context *context) {
    assert(context != nullptr);
    return string_to_view(&(context->options.source));
}

StringView context_assembly_path(Context *context) {
    assert(context != nullptr);
    return string_to_view(&context->options.assembly);
}

StringView context_object_path(Context *context) {
    assert(context != nullptr);
    return string_to_view(&context->options.object);
}

StringView context_output_path(Context *context) {
    assert(context != nullptr);
    return string_to_view(&(context->options.output));
}

Error *context_current_error(Context *context) {
    assert(context != nullptr);
    return &context->current_error;
}

bool context_has_error(Context *context) {
    assert(context != nullptr);
    return context->current_error.code != ERROR_NONE;
}

StringView context_intern(Context *context, StringView sv) {
    assert(context != nullptr);
    return string_interner_insert(&(context->string_interner), sv);
}

Type const *context_nil_type(Context *context) {
    assert(context != nullptr);
    return type_interner_nil_type(&(context->type_interner));
}

Type const *context_boolean_type(Context *context) {
    assert(context != nullptr);
    return type_interner_boolean_type(&(context->type_interner));
}

Type const *context_i32_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i32_type(&(context->type_interner));
}

Type const *context_tuple_type(Context *context, TupleType tuple) {
    assert(context != nullptr);
    return type_interner_tuple_type(&context->type_interner, tuple);
}

Type const *context_function_type(Context *context,
                                  Type const *return_type,
                                  TupleType argument_types) {
    assert(context != nullptr);
    return type_interner_function_type(
        &context->type_interner, return_type, argument_types);
}

Operand context_labels_insert(Context *context, StringView symbol) {
    assert(context != nullptr);
    return labels_insert(&context->labels, symbol);
}

StringView context_labels_at(Context *context, u32 index) {
    assert(context != nullptr);
    return labels_at(&context->labels, index);
}

Symbol *context_symbol_table_at(Context *context, StringView name) {
    assert(context != nullptr);
    return symbol_table_at(&context->symbol_table, name);
}

void context_enter_function(Context *c, FunctionBody *body) {
    assert(c != nullptr);
    assert(body != nullptr);
    c->current_function = body;
}

void context_leave_function(Context *c) {
    assert(c != nullptr);
    c->current_function = nullptr;
}

FunctionBody *context_current_function(Context *c) {
    assert(c != nullptr);
    assert(c->current_function != nullptr);
    return c->current_function;
}

Block *context_current_block(Context *c) {
    assert(c != nullptr);
    return &(context_current_function(c)->block);
}

/*
static Operand context_new_ssa(Context *c) {
    assert(c != nullptr);
    return function_body_new_ssa(context_current_function(c));
}

void context_def_local_const(Context *c, StringView name, Operand value) {
    assert(c != nullptr);
    Operand A = context_emit_load(c, value);
    assert(A.kind == OPERAND_KIND_SSA);
    function_body_new_local(context_current_function(c), name, A.data.ssa);
}

LocalVariable *context_lookup_local(Context *c, StringView name) {
    assert(c != nullptr);
    return function_body_locals_lookup(context_current_function(c), name);
}

LocalVariable *context_lookup_ssa(Context *c, u32 ssa) {
    assert(c != nullptr);
    return function_body_locals_ssa(context_current_function(c), ssa);
}

FormalArgument *context_lookup_argument(Context *c, StringView name) {
    assert(c != nullptr);
    return function_body_arguments_lookup(context_current_function(c), name);
}

FormalArgument *context_argument_at(Context *c, u8 index) {
    assert(c != nullptr);
    return function_body_arguments_at(context_current_function(c), index);
}
*/

Operand context_constants_append(Context *context, Value value) {
    assert(context != nullptr);
    return constants_append(&(context->constants), value);
}

Value *context_constants_at(Context *context, u32 index) {
    assert(context != nullptr);
    return constants_at(&(context->constants), index);
}

void context_emit_return(Context *c, Operand B) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    block_append(bc, instruction_return(B));
}

Operand context_emit_call(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    Operand A = context_new_ssa(c);
    block_append(bc, instruction_call(A, B, C));
    return A;
}

Operand context_emit_dot(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    Operand A = context_new_ssa(c);
    block_append(bc, instruction_dot(A, B, C));
    return A;
}

Operand context_emit_load(Context *c, Operand B) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    Operand A = context_new_ssa(c);
    block_append(bc, instruction_load(A, B));
    return A;
}

Operand context_emit_negate(Context *c, Operand B) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    Operand A = context_new_ssa(c);
    block_append(bc, instruction_negate(A, B));
    return A;
}

Operand context_emit_add(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    Operand A = context_new_ssa(c);
    block_append(bc, instruction_add(A, B, C));
    return A;
}

Operand context_emit_subtract(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    Operand A = context_new_ssa(c);
    block_append(bc, instruction_subtract(A, B, C));
    return A;
}

Operand context_emit_multiply(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    Operand A = context_new_ssa(c);
    block_append(bc, instruction_multiply(A, B, C));
    return A;
}

Operand context_emit_divide(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    Operand A = context_new_ssa(c);
    block_append(bc, instruction_divide(A, B, C));
    return A;
}

Operand context_emit_modulus(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Block *bc = context_current_block(c);
    Operand A = context_new_ssa(c);
    block_append(bc, instruction_modulus(A, B, C));
    return A;
}

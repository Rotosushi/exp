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

#include "env/context.h"

Context context_create(CLIOptions *options) {
    assert(options != nullptr);
    Context context = {.options             = context_options_create(options),
                       .string_interner     = string_interner_create(),
                       .type_interner       = type_interner_create(),
                       .global_symbol_table = symbol_table_create(),
                       .global_labels       = labels_create(),
                       .constants           = constants_create(),
                       .current_error       = error_create(),
                       .current_function    = nullptr};
    return context;
}

void context_destroy(Context *context) {
    assert(context != nullptr);
    context_options_destroy(&(context->options));
    string_interner_destroy(&(context->string_interner));
    type_interner_destroy(&(context->type_interner));
    symbol_table_destroy(&(context->global_symbol_table));
    labels_destroy(&(context->global_labels));
    constants_destroy(&(context->constants));
    error_destroy(&context->current_error);
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

Type *context_nil_type(Context *context) {
    assert(context != nullptr);
    return type_interner_nil_type(&(context->type_interner));
}

Type *context_boolean_type(Context *context) {
    assert(context != nullptr);
    return type_interner_boolean_type(&(context->type_interner));
}

Type *context_i64_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i64_type(&(context->type_interner));
}

Type *context_tuple_type(Context *context, TupleType tuple) {
    assert(context != nullptr);
    return type_interner_tuple_type(&context->type_interner, tuple);
}

Type *context_function_type(Context *context,
                            Type *return_type,
                            TupleType argument_types) {
    assert(context != nullptr);
    return type_interner_function_type(
        &context->type_interner, return_type, argument_types);
}

u16 context_labels_insert(Context *context, StringView symbol) {
    assert(context != nullptr);
    return labels_insert(&context->global_labels, symbol);
}

StringView context_labels_at(Context *context, u16 index) {
    assert(context != nullptr);
    return labels_at(&context->global_labels, index);
}

Symbol *context_global_symbol_table_at(Context *context, StringView name) {
    assert(context != nullptr);
    return symbol_table_at(&context->global_symbol_table, name);
}

SymbolTableIterator context_global_symbol_table_iterator(Context *context) {
    assert(context != nullptr);
    return symbol_table_iterator_create(&context->global_symbol_table);
}

FunctionBody *context_enter_function(Context *c, StringView name) {
    assert(c != nullptr);
    Symbol *element = symbol_table_at(&c->global_symbol_table, name);
    if (element->kind == STE_UNDEFINED) { element->kind = STE_FUNCTION; }

    c->current_function = &element->function_body;
    return c->current_function;
}

FunctionBody *context_current_function(Context *c) {
    assert(c != nullptr);
    assert(c->current_function != nullptr);
    return c->current_function;
}

Bytecode *context_active_bytecode(Context *c) {
    return &(context_current_function(c)->bc);
}

static Operand context_new_ssa(Context *c) {
    return function_body_new_ssa(context_current_function(c));
}

void context_def_local_const(Context *c, StringView name, Operand value) {
    Operand A = context_emit_load(c, value);
    assert(A.kind == OPERAND_KIND_SSA);
    function_body_new_local(context_current_function(c), name, A.data.ssa);
}

LocalVariable *context_lookup_local(Context *c, StringView name) {
    return local_variables_lookup(&(context_current_function(c)->locals), name);
}

LocalVariable *context_lookup_ssa(Context *c, u16 ssa) {
    return local_variables_lookup_ssa(&(context_current_function(c)->locals),
                                      ssa);
}

FormalArgument *context_lookup_argument(Context *c, StringView name) {
    return formal_argument_list_lookup(
        &(context_current_function(c)->arguments), name);
}

FormalArgument *context_argument_at(Context *c, u8 index) {
    return formal_argument_list_at(&(context_current_function(c)->arguments),
                                   index);
}

void context_leave_function(Context *c) {
    assert(c != nullptr);
    c->current_function = nullptr;
}

Operand context_constants_append(Context *context, Value value) {
    assert(context != nullptr);
    return constants_append(&(context->constants), value);
}

Value *context_constants_at(Context *context, u16 index) {
    assert(context != nullptr);
    return constants_at(&(context->constants), index);
}

void context_emit_return(Context *c, Operand B) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    bytecode_append(bc, instruction_return(B));
}

Operand context_emit_call(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    Operand A    = context_new_ssa(c);
    bytecode_append(bc, instruction_call(A, B, C));
    return A;
}

Operand context_emit_dot(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    Operand A    = context_new_ssa(c);
    bytecode_append(bc, instruction_dot(A, B, C));
    return A;
}

Operand context_emit_load(Context *c, Operand B) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    Operand A    = context_new_ssa(c);
    bytecode_append(bc, instruction_load(A, B));
    return A;
}

Operand context_emit_negate(Context *c, Operand B) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    Operand A    = context_new_ssa(c);
    bytecode_append(bc, instruction_negate(A, B));
    return A;
}

Operand context_emit_add(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    Operand A    = context_new_ssa(c);
    bytecode_append(bc, instruction_add(A, B, C));
    return A;
}

Operand context_emit_subtract(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    Operand A    = context_new_ssa(c);
    bytecode_append(bc, instruction_subtract(A, B, C));
    return A;
}

Operand context_emit_multiply(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    Operand A    = context_new_ssa(c);
    bytecode_append(bc, instruction_multiply(A, B, C));
    return A;
}

Operand context_emit_divide(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    Operand A    = context_new_ssa(c);
    bytecode_append(bc, instruction_divide(A, B, C));
    return A;
}

Operand context_emit_modulus(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Bytecode *bc = context_active_bytecode(c);
    Operand A    = context_new_ssa(c);
    bytecode_append(bc, instruction_modulus(A, B, C));
    return A;
}

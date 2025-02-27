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

void context_initialize(Context *context,
                        Bitset flags,
                        StringView source,
                        StringView output) {
    assert(context != nullptr);
    context_options_initialize(&context->options, flags, source, output);
    string_interner_initialize(&context->string_interner);
    type_interner_initialize(&context->type_interner);
    symbol_table_create(&context->symbol_table);
    // labels_initialize(&context->labels);
    constants_initialize(&context->constants);
    error_initialize(&context->current_error);
}

void context_terminate(Context *context) {
    assert(context != nullptr);
    context_options_terminate(&(context->options));
    string_interner_terminate(&(context->string_interner));
    type_interner_destroy(&(context->type_interner));
    symbol_table_destroy(&(context->symbol_table));
    // labels_terminate(&(context->labels));
    constants_terminate(&(context->constants));
    error_terminate(&context->current_error);
}

bool context_emit_ir_assembly(Context *context) {
    assert(context != nullptr);
    return context_options_emit_ir_assembly(&context->options);
}

bool context_emit_x86_64_assembly(Context *context) {
    assert(context != nullptr);
    return context_options_emit_x86_64_assembly(&context->options);
}

bool context_create_elf_object(Context *context) {
    assert(context != nullptr);
    return context_options_create_elf_object(&context->options);
}

bool context_create_elf_executable(Context *context) {
    assert(context != nullptr);
    return context_options_create_elf_executable(&context->options);
}

bool context_cleanup_x86_64_assembly(Context *context) {
    assert(context != nullptr);
    return context_options_cleanup_target_assembly(&context->options);
}

bool context_cleanup_elf_object(Context *context) {
    assert(context != nullptr);
    return context_options_cleanup_elf_object(&context->options);
}

StringView context_source_path(Context *context) {
    assert(context != nullptr);
    return string_to_view(&(context->options.source));
}

StringView context_ir_path(Context *context) {
    assert(context != nullptr);
    return string_to_view(&context->options.ir_assembly);
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

ConstantString *context_intern(Context *context, StringView sv) {
    assert(context != nullptr);
    return string_interner_insert(&(context->string_interner), sv);
}

/*
Type const *context_nil_type(Context *context) {
    assert(context != nullptr);
    return type_interner_nil_type(&(context->type_interner));
}

Type const *context_boolean_type(Context *context) {
    assert(context != nullptr);
    return type_interner_boolean_type(&(context->type_interner));
}
*/

Type const *context_i64_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i64_type(&(context->type_interner));
}

Type const *context_tuple_type(Context *context, TupleType tuple) {
    assert(context != nullptr);
    return type_interner_tuple_type(&context->type_interner, tuple);
}

Type const *context_function_type(Context *context,
                                  Type const *return_type,
                                  TupleType argument_types) {
    assert(context != nullptr);
    assert(return_type != nullptr);
    return type_interner_function_type(
        &context->type_interner, return_type, argument_types);
}

Symbol *context_symbol_table_at(Context *context, StringView name) {
    assert(context != nullptr);
    return symbol_table_at(&context->symbol_table, name);
}

Value *context_constants_append_tuple(Context *context, Tuple tuple) {
    assert(context != nullptr);
    return constants_append_tuple(&(context->constants), tuple);
}

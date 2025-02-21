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
 * @file env/context.c
 */
#include "env/context.h"
#include "imr/locals.h"
#include "utility/assert.h"

void context_initialize(Context *context, Bitset flags, StringView source,
                        StringView output) {
    EXP_ASSERT(context != nullptr);
    context_options_initialize(&context->options, flags, source, output);
    string_interner_initialize(&context->string_interner);
    type_interner_initialize(&context->type_interner);
    symbol_table_create(&context->symbol_table);
    locals_initialize(&context->locals);
    frames_initialize(&context->frames);
    stack_initialize(&context->stack);
    error_initialize(&context->current_error);
}

void context_terminate(Context *context) {
    EXP_ASSERT(context != nullptr);
    context_options_terminate(&(context->options));
    string_interner_terminate(&(context->string_interner));
    type_interner_destroy(&(context->type_interner));
    symbol_table_destroy(&(context->symbol_table));
    locals_terminate(&context->locals);
    frames_terminate(&context->frames);
    stack_terminate(&context->stack);
    error_terminate(&context->current_error);
}

bool context_emit_ir_assembly(Context *context) {
    EXP_ASSERT(context != nullptr);
    return context_options_emit_ir_assembly(&context->options);
}

bool context_emit_x86_64_assembly(Context *context) {
    EXP_ASSERT(context != nullptr);
    return context_options_emit_x86_64_assembly(&context->options);
}

bool context_create_elf_object(Context *context) {
    EXP_ASSERT(context != nullptr);
    return context_options_create_elf_object(&context->options);
}

bool context_create_elf_executable(Context *context) {
    EXP_ASSERT(context != nullptr);
    return context_options_create_elf_executable(&context->options);
}

bool context_cleanup_x86_64_assembly(Context *context) {
    EXP_ASSERT(context != nullptr);
    return context_options_cleanup_target_assembly(&context->options);
}

bool context_cleanup_elf_object(Context *context) {
    EXP_ASSERT(context != nullptr);
    return context_options_cleanup_elf_object(&context->options);
}

StringView context_source_path(Context *context) {
    EXP_ASSERT(context != nullptr);
    return string_to_view(&(context->options.source));
}

StringView context_ir_path(Context *context) {
    EXP_ASSERT(context != nullptr);
    return string_to_view(&context->options.ir_assembly);
}

StringView context_assembly_path(Context *context) {
    EXP_ASSERT(context != nullptr);
    return string_to_view(&context->options.assembly);
}

StringView context_object_path(Context *context) {
    EXP_ASSERT(context != nullptr);
    return string_to_view(&context->options.object);
}

StringView context_output_path(Context *context) {
    EXP_ASSERT(context != nullptr);
    return string_to_view(&(context->options.output));
}

Error *context_current_error(Context *context) {
    EXP_ASSERT(context != nullptr);
    return &context->current_error;
}

bool context_has_error(Context *context) {
    EXP_ASSERT(context != nullptr);
    return context->current_error.code != ERROR_NONE;
}

StringView context_intern(Context *context, StringView sv) {
    EXP_ASSERT(context != nullptr);
    return string_interner_insert(&(context->string_interner), sv);
}

Type const *context_nil_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_nil_type(&(context->type_interner));
}

Type const *context_bool_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_boolean_type(&(context->type_interner));
}

Type const *context_i8_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_i8_type(&(context->type_interner));
}

Type const *context_i16_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_i16_type(&(context->type_interner));
}

Type const *context_i32_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_i32_type(&(context->type_interner));
}

Type const *context_i64_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_i64_type(&(context->type_interner));
}

Type const *context_u8_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_u8_type(&(context->type_interner));
}

Type const *context_u16_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_u16_type(&(context->type_interner));
}

Type const *context_u32_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_u32_type(&(context->type_interner));
}

Type const *context_u64_type(Context *context) {
    EXP_ASSERT(context != nullptr);
    return type_interner_u64_type(&(context->type_interner));
}

Type const *context_tuple_type(Context *context, TupleType tuple) {
    EXP_ASSERT(context != nullptr);
    return type_interner_tuple_type(&context->type_interner, tuple);
}

Type const *context_function_type(Context *context, Type const *return_type,
                                  TupleType argument_types) {
    EXP_ASSERT(context != nullptr);
    EXP_ASSERT(return_type != nullptr);
    return type_interner_function_type(&context->type_interner, return_type,
                                       argument_types);
}

Symbol *context_symbol_table_at(Context *context, StringView name) {
    EXP_ASSERT(context != nullptr);
    return symbol_table_at(&context->symbol_table, name);
}

Frame *context_frames_top(Context *context) {
    EXP_ASSERT(context != nullptr);
    return frames_top(&context->frames);
}

Frame *context_frames_push(Context *context, Function *function, u32 base) {
    EXP_ASSERT(context != nullptr);
    return frames_push(&context->frames, function, base);
}

void context_frames_pop(Context *context) {
    EXP_ASSERT(context != nullptr);
    frames_pop(&context->frames);
}

u32 context_stack_length(Context *context) {
    EXP_ASSERT(context != nullptr);
    return context->stack.length;
}

Value *context_stack_top(Context *context) {
    EXP_ASSERT(context != nullptr);
    return stack_top(&context->stack);
}

Value *context_stack_peek(Context *context, u32 n) {
    EXP_ASSERT(context != nullptr);
    return stack_peek(&context->stack, n);
}

u32 context_stack_push(Context *context, Value value) {
    EXP_ASSERT(context != nullptr);
    return stack_push(&context->stack, value);
}

Value context_stack_pop(Context *context) {
    EXP_ASSERT(context != nullptr);
    return stack_pop(&context->stack);
}

void context_stack_pop_n(Context *context, u32 n) {
    EXP_ASSERT(context != nullptr);
    stack_pop_n(&context->stack, n);
}

bool context_registers_next_available(Context *context, u8 *register_) {
    EXP_ASSERT(context != nullptr);
    return registers_next_available(&context->registers, register_);
}

void context_registers_set(Context *context, u8 register_, Scalar value) {
    EXP_ASSERT(context != nullptr);
    registers_set(&context->registers, register_, value);
}

Scalar context_registers_get(Context *context, u8 register_) {
    EXP_ASSERT(context != nullptr);
    return registers_get(&context->registers, register_);
}

Scalar context_registers_unset(Context *context, u8 register_) {
    EXP_ASSERT(context != nullptr);
    return registers_unset(&context->registers, register_);
}

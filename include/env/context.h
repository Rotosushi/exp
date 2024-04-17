// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_ENV_CONTEXT_H
#define EXP_ENV_CONTEXT_H

#include "env/context_options.h"
#include "env/string_interner.h"
#include "env/symbol_table.h"
#include "env/type_interner.h"
#include "imr/bytecode.h"
#include "imr/constants.h"
// #include "imr/registers.h"
#include "imr/stack.h"

/**
 * @brief A context models a Translation Unit.
 * it's just that context is a shorter type name than
 * TranslationUnit
 *
 */
typedef struct Context {
  ContextOptions options;
  StringInterner string_interner;
  TypeInterner type_interner;
  SymbolTable global_symbols;
  Bytecode global_bytecode;
  Bytecode *current_function_body;
  Constants constants;
  Stack stack;
  // Registers registers;
} Context;

/**
 * @brief create a new options
 *
 * @note takes ownership of the options passed in.
 *
 * @param options
 * @return Context
 */
Context context_create(ContextOptions *restrict options);
void context_destroy(Context *restrict context);

// context options functions
StringView context_source_path(Context *restrict context);

FILE *context_open_source(Context *restrict context);

String context_buffer_source(Context *restrict context);

StringView context_output_path(Context *restrict context);

FILE *context_open_output(Context *restrict context);

// string interner functions
StringView context_intern(Context *restrict context, StringView sv);

// type interner functions
Type *context_nil_type(Context *restrict context);

Type *context_boolean_type(Context *restrict context);

Type *context_integer_type(Context *restrict context);

Type *context_string_literal_type(Context *restrict context);

Type *context_function_type(Context *restrict context, Type *return_type,
                            ArgumentTypes argument_types);

// symbol table functions
bool context_insert_global_symbol(Context *restrict context, StringView name,
                                  Type *type, Value *value);

SymbolTableElement *context_lookup_global_symbol(Context *restrict context,
                                                 StringView name);

// Constants functions
u64 context_constants_append(Context *restrict context, Value value);

Value *context_constants_at(Context *restrict context, u64 index);

// Stack functions
bool context_stack_empty(Context *restrict context);

void context_stack_push(Context *restrict context, Value *value);

Value *context_stack_pop(Context *restrict context);

Value *context_stack_peek(Context *restrict context);

// Bytecode functions

/**
 * @brief sets the current function body to emit bytecode into to <body>
 * and returns the previous function body.
 *
 * @note the current function starts as NULL and this means
 * bytecode will be emitted into the global bytecode. So this
 * function may return NULL, and <body> is allowed to be NULL.
 *
 * @param context
 * @param body
 * @return Bytecode*
 */
Bytecode *context_current_function_body(Context *restrict context,
                                        Bytecode *restrict body);

Bytecode *context_current_bytecode(Context *restrict context);

/**
 * @brief reads an immediate from the current bytecode
 *
 * @param context
 * @param offset
 * @param bytes
 * @return u64
 */
u64 context_read_immediate(Context *restrict context, u64 offset, u64 bytes);

void context_emit_stop(Context *restrict context);

void context_emit_return(Context *restrict context);

void context_emit_push_constant(Context *restrict context, u64 index);

void context_emit_define_global_constant(Context *restrict context);

void context_emit_unop_minus(Context *restrict context);

void context_emit_binop_plus(Context *restrict context);

void context_emit_binop_minus(Context *restrict context);

void context_emit_binop_star(Context *restrict context);

void context_emit_binop_slash(Context *restrict context);

void context_emit_binop_percent(Context *restrict context);
#endif // !EXP_ENV_CONTEXT_H
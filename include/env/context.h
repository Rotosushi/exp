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

typedef struct Context {
  ContextOptions options;
  StringInterner string_interner;
  TypeInterner type_interner;
  SymbolTable global_symbols;
  Bytecode global_bytecode;
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

// symbol table functions
bool context_insert_global_symbol(Context *restrict context, StringView name,
                                  Type *type, Value value);

SymbolTableElement *context_lookup_global_symbol(Context *restrict context,
                                                 StringView name);

// Constants functions
size_t context_constants_append(Context *restrict context, Value value);

Value *context_constants_at(Context *restrict context, size_t index);

// Stack functions
bool context_stack_empty(Context *restrict context);

void context_stack_push(Context *restrict context, Value value);

Value context_stack_pop(Context *restrict context);

Value *context_stack_peek(Context *restrict context);

// global Bytecode functions
void context_emit_stop(Context *restrict context);

void context_emit_push_constant(Context *restrict context, size_t index);

void context_emit_define_global_constant(Context *restrict context);

#endif // !EXP_ENV_CONTEXT_H
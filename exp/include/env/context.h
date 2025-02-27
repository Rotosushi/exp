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

#include "env/constants.h"
#include "env/context_options.h"
#include "env/error.h"
#include "env/labels.h"
#include "env/string_interner.h"
#include "env/symbol_table.h"
#include "env/type_interner.h"
#include "imr/function_body.h"

/**
 * @brief A context models a Translation Unit.
 *
 */
typedef struct Context {
    ContextOptions options;
    StringInterner string_interner;
    TypeInterner type_interner;
    SymbolTable symbol_table;
    Labels labels;
    Constants constants;
    Error current_error;
    FunctionBody *current_function;
} Context;

/**
 * @brief create a new options
 *
 * @note takes ownership of the options passed in.
 *
 * @param options
 * @return Context
 */
void context_initialize(Context *context, CLIOptions *options);
void context_terminate(Context *context);

// context options functions
bool context_do_assemble(Context *context);
bool context_do_link(Context *context);
bool context_do_cleanup(Context *context);

StringView context_source_path(Context *context);
StringView context_assembly_path(Context *context);
StringView context_object_path(Context *context);
StringView context_output_path(Context *context);

// current error functions
Error *context_current_error(Context *context);
bool context_has_error(Context *context);

// string interner functions
StringView context_intern(Context *context, StringView sv);

// type interner functions
Type const *context_nil_type(Context *context);
Type const *context_boolean_type(Context *context);
Type const *context_i64_type(Context *context);
Type const *context_tuple_type(Context *context, TupleType tuple);
Type const *context_function_type(Context *context,
                                  Type const *return_type,
                                  TupleType argument_types);

// labels functions
Operand context_labels_insert(Context *context, StringView symbol);
StringView context_labels_at(Context *context, u16 index);

// symbol table functions
Symbol *context_symbol_table_at(Context *context, StringView name);

// function functions
void context_enter_function(Context *c, FunctionBody *body);
void context_leave_function(Context *c);
FunctionBody *context_current_function(Context *c);
Block *context_current_block(Context *c);

// CallPair context_new_call(Context * c);
// ActualArgumentList *context_call_at(Context * c, u64 idx);

void context_def_local_const(Context *c, StringView name, Operand value);

LocalVariable *context_lookup_local(Context *c, StringView name);
LocalVariable *context_lookup_ssa(Context *c, u16 ssa);

FormalArgument *context_lookup_argument(Context *c, StringView name);
FormalArgument *context_argument_at(Context *c, u8 index);

// Values functions
Operand context_constants_append(Context *context, Value value);
Value *context_constants_at(Context *context, u16 index);

// Bytecode functions
void context_emit_return(Context *c, Operand B);
Operand context_emit_call(Context *c, Operand B, Operand C);
Operand context_emit_dot(Context *c, Operand B, Operand C);
Operand context_emit_load(Context *c, Operand B);
Operand context_emit_negate(Context *c, Operand B);
Operand context_emit_add(Context *c, Operand B, Operand C);
Operand context_emit_subtract(Context *c, Operand B, Operand C);
Operand context_emit_multiply(Context *c, Operand B, Operand C);
Operand context_emit_divide(Context *c, Operand B, Operand C);
Operand context_emit_modulus(Context *c, Operand B, Operand C);

#endif // !EXP_ENV_CONTEXT_H

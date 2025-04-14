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

/**
 * @brief A context models a Translation Unit.
 *
 */
typedef struct Context {
    ContextOptions options;
    String         source_path;
    String         ir_path;
    String         assembly_path;
    String         object_path;
    String         executable_path;
    String         library_path;
    StringInterner string_interner;
    TypeInterner   type_interner;
    SymbolTable    global_symbol_table;
    Labels         global_labels;
    Constants      constants;
    Error          current_error;
    Function      *current_function;
} Context;

/**
 * @brief create a new options
 *
 * @note takes ownership of the options passed in.
 *
 * @param options
 * @return Context
 */
void context_create(Context *restrict context,
                    ContextOptions *restrict options,
                    StringView source_path);
void context_destroy(Context *restrict context);

// context options functions
bool context_prolix(Context const *context);
bool context_trace(Context const *restrict context);
bool context_create_ir_artifact(Context const *restrict context);
bool context_create_assembly_artifact(Context const *restrict context);
bool context_create_object_artifact(Context const *restrict context);
bool context_create_executable_artifact(Context const *restrict context);
bool context_cleanup_ir_artifact(Context const *restrict context);
bool context_cleanup_assembly_artifact(Context const *restrict context);
bool context_cleanup_object_artifact(Context const *restrict context);

StringView context_source_path(Context const *restrict context);
StringView context_ir_path(Context const *restrict context);
StringView context_assembly_path(Context const *restrict context);
StringView context_object_path(Context const *restrict context);
StringView context_executable_path(Context const *restrict context);

// current error functions
Error *context_current_error(Context *restrict context);
bool   context_has_error(Context const *restrict context);

// string interner functions
StringView context_intern(Context *restrict context, StringView sv);

// type interner functions
Type const *context_nil_type(Context *restrict context);
Type const *context_boolean_type(Context *restrict context);
Type const *context_u8_type(Context *restrict context);
Type const *context_u16_type(Context *restrict context);
Type const *context_u32_type(Context *restrict context);
Type const *context_u64_type(Context *restrict context);
Type const *context_i8_type(Context *restrict context);
Type const *context_i16_type(Context *restrict context);
Type const *context_i32_type(Context *restrict context);
Type const *context_i64_type(Context *restrict context);
Type const *context_tuple_type(Context *restrict context, TupleType tuple);
Type const *context_function_type(Context *restrict context,
                                  Type const *return_type,
                                  TupleType   argument_types);

// labels functions
u32        context_labels_insert(Context *restrict context, StringView symbol);
StringView context_labels_at(Context *restrict context, u32 index);

// symbol table functions
Symbol *context_global_symbol_table_at(Context *restrict context,
                                       StringView name);

// function functions
Function *context_enter_function(Context *restrict context, StringView name);
Function *context_current_function(Context *restrict context);
Bytecode *context_active_bytecode(Context *restrict context);

// CallPair context_new_call(Context * c);
// ActualArgumentList *context_call_at(Context * c, u64 idx);

void context_def_local_const(Context *restrict context,
                             StringView name,
                             Operand    value);

LocalVariable *context_lookup_local(Context *restrict context, StringView name);
LocalVariable *context_lookup_ssa(Context *restrict context, u32 ssa);

FormalArgument *context_lookup_argument(Context *restrict context,
                                        StringView name);
FormalArgument *context_argument_at(Context *restrict context, u8 index);

void context_leave_function(Context *restrict context);

// Values functions
Operand context_constants_append(Context *restrict context, Value value);
Value  *context_constants_at(Context *restrict context, u32 index);

// Bytecode functions
void    context_emit_return(Context *restrict context, Operand B);
Operand context_emit_call(Context *restrict context, Operand B, Operand C);
Operand context_emit_dot(Context *restrict context, Operand B, Operand C);
Operand context_emit_load(Context *restrict context, Operand B);
Operand context_emit_negate(Context *restrict context, Operand B);
Operand context_emit_add(Context *restrict context, Operand B, Operand C);
Operand context_emit_subtract(Context *restrict context, Operand B, Operand C);
Operand context_emit_multiply(Context *restrict context, Operand B, Operand C);
Operand context_emit_divide(Context *restrict context, Operand B, Operand C);
Operand context_emit_modulus(Context *restrict context, Operand B, Operand C);

#endif // !EXP_ENV_CONTEXT_H

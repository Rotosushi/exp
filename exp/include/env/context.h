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
#include "env/frames.h"
#include "env/stack.h"
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
    StringInterner string_interner;
    TypeInterner   type_interner;
    SymbolTable    global_symbol_table;
    Constants      constants;
    Stack          stack;
    Frames         frames;
    Error          current_error;
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
                    ContextOptions *restrict options);
void context_destroy(Context *restrict context);

// context options functions
bool context_shall_prolix(Context const *restrict context);
bool context_shall_create_assembly_artifact(Context const *restrict context);
bool context_shall_create_object_artifact(Context const *restrict context);
bool context_shall_create_executable_artifact(Context const *restrict context);
bool context_shall_cleanup_assembly_artifact(Context const *restrict context);
bool context_shall_cleanup_object_artifact(Context const *restrict context);

void context_print_compile_actions(Context const *restrict context);

void *context_get_target_context(Context const *restrict context);

// Compilation Helpers
i32 context_compile_source(Context *restrict context, StringView source);
// #TODO: i32  context_repl(Context *restrict context);
i32  context_create_assembly_artifact(Context *restrict context);
i32  context_create_object_artifact(Context *restrict context);
i32  context_create_executable_artifact(Context *restrict context);
void context_cleanup_assembly_artifact(Context *restrict context);
void context_cleanup_object_artifact(Context *restrict context);

StringView context_source_path(Context const *restrict context);

// Failure functions
void context_print_error(Context const *restrict context,
                         StringView file,
                         u64        line);
bool context_failure(Context *restrict context,
                     ErrorCode  code,
                     StringView message);
bool context_failure_string(Context *restrict context,
                            ErrorCode code,
                            String    string);
bool context_failure_type_is_not_callable(Context *restrict context,
                                          Type const *type);
bool context_failure_type_is_not_indexable(Context *restrict context,
                                           Type const *type);
bool context_failure_operand_is_not_an_index(Context *restrict context,
                                             Operand operand);
bool context_failure_index_out_of_bounds(Context *restrict context,
                                         u64 bound,
                                         u64 index);
bool context_failure_mismatch_argument_count(Context *restrict context,
                                             u64 expected,
                                             u64 actual);
bool context_failure_mismatch_type(Context *restrict context,
                                   Type const *expected,
                                   Type const *actual);
bool context_failure_uninitialized_value(Context *restrict context);
bool context_failure_undefined_symbol(Context *restrict context,
                                      StringView symbol);
bool context_failure_unsupported_operand(Context *restrict context,
                                         StringView operator,
                                         Operand operand);
bool context_failure_unsupported_operand_value(Context *restrict context,
                                               StringView operator,
                                               Value const * restrict value);
bool context_failure_unsigned_overflow(Context *restrict context,
                                       StringView operator,
                                       Type const * type,
                                       u64 A,
                                       u64 B);
bool context_failure_signed_overflow(Context *restrict context,
                                     StringView operator,
                                     Type const * type,
                                     i64 A,
                                     i64 B);

// string interner functions
ConstantString *context_intern(Context *restrict context, StringView sv);

// type interner functions
Type const *context_nil_type(Context *restrict context);
Type const *context_bool_type(Context *restrict context);
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

// Constants functions
Value const *context_constant_nil(Context *restrict context);
Value const *context_constant_true(Context *restrict context);
Value const *context_constant_false(Context *restrict context);
Value const *context_constant_u8(Context *restrict context, u8 u8_);
Value const *context_constant_u16(Context *restrict context, u16 u16_);
Value const *context_constant_u32(Context *restrict context, u32 u32_);
Value const *context_constant_u64(Context *restrict context, u64 u64_);
Value const *context_constant_i8(Context *restrict context, i8 i8_);
Value const *context_constant_i16(Context *restrict context, i16 i16_);
Value const *context_constant_i32(Context *restrict context, i32 i32_);
Value const *context_constant_i64(Context *restrict context, i64 i64_);
Value const *context_constant_tuple(Context *restrict context, Value *tuple);
Value const *context_constant_function(Context *restrict context,
                                       Value *function);

// Stack functions
bool context_stack_empty(Context const *restrict context);
u32  context_stack_size(Context const *restrict context);
void context_stack_push(Context *restrict context, Value const *restrict value);
Value const *context_stack_pop(Context *restrict context);
void         context_stack_pop_n(Context *restrict context, u32 n);
Value const *
context_stack_peek(Context const *restrict context, u32 offset, u32 index);

// Frames functions
bool   context_frames_empty(Context const *restrict context);
void   context_frames_push(Context *restrict context, Frame frame);
Frame *context_frames_top(Context const *restrict context);
void   context_frames_pop(Context *restrict context);

// symbol table functions
Symbol *context_global_symbol_lookup(Context *restrict context,
                                     StringView name);

// Type checking helpers
Type const *context_type_of_value(Context *restrict context,
                                  Function const *restrict function,
                                  Value const *restrict value);

Type const *context_type_of_function(Context *restrict context,
                                     Function const *restrict function);

Type const *context_type_of_tuple(Context *restrict context,
                                  Function const *restrict function,
                                  Tuple const *restrict tuple);

Type const *context_type_of_operand(Context *restrict context,
                                    Function const *restrict function,
                                    Operand operand);

u64 context_size_of(Context *restrict context, Type const *type);
u64 context_align_of(Context *restrict context, Type const *type);

// Evaluation helpers
bool context_at_top_level(Context const *restrict context);

typedef enum LookupResultKind {
    LOOKUP_RESULT_NONE,
    LOOKUP_RESULT_LOCAL,
    LOOKUP_RESULT_GLOBAL,
} LookupResultKind;

typedef struct LookupResult {
    LookupResultKind kind;
    union {
        Local  *local;
        Symbol *global;
    };
} LookupResult;

LookupResult context_lookup_label(Context *restrict context,
                                  Function const *restrict function,
                                  StringView label);

void context_push_local_value(Context *restrict context,
                              Frame *restrict frame,
                              Local *restrict local,
                              Value const *restrict constant);
// void context_create_callee_frame(Context *restrict context,
//                                  Function const *restrict function);
// void context_allocate_local(Context *restrict context,
//                             Local *restrict local,
//                             Value const *value);
// void context_destroy_frame(Context *restrict context);

// function functions
// Function *context_enter_function(Context *restrict context, StringView name);
// Function *context_current_function(Context *restrict context);
// Bytecode *context_active_bytecode(Context *restrict context);

// // CallPair context_new_call(Context * c);
// // ActualArgumentList *context_call_at(Context * c, u64 idx);

// Local *context_declare_argument(Context *restrict context);
// Local *context_declare_local(Context *restrict context);
// Local *context_lookup_argument(Context *restrict context, u8 index);
// Local *context_lookup_local(Context *restrict context, u32 ssa);
// Local *context_lookup_local_name(Context *restrict context, StringView name);

// void context_leave_function(Context *restrict context);

// // Bytecode functions
// void    context_emit_return(Context *restrict context, Operand B);
// Operand context_emit_call(Context *restrict context, Operand B, Operand C);
// Operand context_emit_dot(Context *restrict context, Operand B, Operand C);
// Operand context_emit_load(Context *restrict context, Operand B);
// Operand context_emit_negate(Context *restrict context, Operand B);
// Operand context_emit_add(Context *restrict context, Operand B, Operand C);
// Operand context_emit_subtract(Context *restrict context, Operand B, Operand
// C); Operand context_emit_multiply(Context *restrict context, Operand B,
// Operand C); Operand context_emit_divide(Context *restrict context, Operand B,
// Operand C); Operand context_emit_modulus(Context *restrict context, Operand
// B, Operand C);

#endif // !EXP_ENV_CONTEXT_H

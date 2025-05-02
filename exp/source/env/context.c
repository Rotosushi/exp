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

#include "codegen/IR/directives.h"
#include "codegen/x86/codegen.h"
#include "codegen/x86/emit.h"
#include "codegen/x86/env/context.h"
#include "env/context.h"
#include "env/context_options.h"
#include "intrinsics/type_of.h"
#include "support/assert.h"
#include "support/config.h"
#include "support/io.h"
#include "support/string_view.h"

#define EXP_IR_EXTENSION  "eir"
#define EXP_ASM_EXTENSION "s"
#define EXP_OBJ_EXTENSION "o"
#define EXP_EXE_EXTENSION ""
#define EXP_LIB_EXTENSION "a"

static void generate_path_from_source(String *restrict target,
                                      StringView source_path,
                                      StringView extension) {
    string_assign(target, source_path);
    string_replace_extension(target, extension);
}

void context_create(Context *restrict context,
                    ContextOptions *restrict options,
                    StringView source_path) {
    assert(context != nullptr);
    assert(options != nullptr);
    assert(!string_view_empty(source_path));
    context->options = *options;
    string_initialize(&(context->source_path));
    string_initialize(&(context->ir_path));
    string_initialize(&(context->assembly_path));
    string_initialize(&(context->object_path));
    string_initialize(&(context->executable_path));
    string_initialize(&(context->library_path));
    string_assign(&(context->source_path), source_path);
    generate_path_from_source(
        &(context->ir_path), source_path, SV(EXP_IR_EXTENSION));
    generate_path_from_source(
        &(context->assembly_path), source_path, SV(EXP_ASM_EXTENSION));
    generate_path_from_source(
        &(context->object_path), source_path, SV(EXP_OBJ_EXTENSION));
    generate_path_from_source(
        &(context->executable_path), source_path, SV(EXP_EXE_EXTENSION));
    generate_path_from_source(
        &(context->library_path), source_path, SV(EXP_LIB_EXTENSION));
    // context->current_function    = nullptr;
    context->current_error       = error_create();
    context->global_symbol_table = symbol_table_create();
    // context->global_labels       = labels_create();
    constants_create(&context->constants);
    stack_create(&context->stack);
    context->string_interner = string_interner_create();
    context->type_interner   = type_interner_create();
}

void context_destroy(Context *context) {
    assert(context != nullptr);
    string_destroy(&(context->source_path));
    string_destroy(&(context->ir_path));
    string_destroy(&(context->assembly_path));
    string_destroy(&(context->object_path));
    string_destroy(&(context->executable_path));
    string_destroy(&(context->library_path));
    string_interner_destroy(&(context->string_interner));
    type_interner_destroy(&(context->type_interner));
    symbol_table_destroy(&(context->global_symbol_table));
    // labels_destroy(&(context->global_labels));
    constants_destroy(&(context->constants));
    stack_destroy(&context->stack);
    error_destroy(&context->current_error);
    // context->current_function = nullptr;
}

bool context_shall_prolix(Context const *context) {
    assert(context != nullptr);
    return context->options.prolix;
}

bool context_shall_create_ir_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.create_ir_artifact;
}
bool context_shall_create_assembly_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.create_assembly_artifact;
}
bool context_shall_create_object_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.create_object_artifact;
}
bool context_shall_create_executable_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.create_executable_artifact;
}
bool context_shall_cleanup_ir_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.cleanup_ir_artifact;
}
bool context_shall_cleanup_assembly_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.cleanup_assembly_artifact;
}
bool context_shall_cleanup_object_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.cleanup_object_artifact;
}

void context_create_ir_artifact(Context *restrict context) {
    assert(context != NULL);

    String ir_path;
    string_initialize(&ir_path);
    generate_path_from_source(
        &ir_path, string_to_view(&context->source_path), SV(EXP_IR_EXTENSION));

    String contents;
    string_initialize(&contents);
    ir_directive_version(SV(EXP_VERSION_STRING), &contents);
    ir_directive_file(string_to_view(&context->source_path), &contents);

    SymbolTable *symbol_table = &context->global_symbol_table;
    for (u64 index = 0; index < symbol_table->capacity; ++index) {
        Symbol *symbol = symbol_table->elements[index];
        if (symbol == NULL) { continue; }

        ir_directive_function(symbol->name, &contents);
        print_value(&contents, symbol->value, context);
        string_append(&contents, SV("\n"));
    }

    FILE *ir_file = file_open(string_to_cstring(&ir_path), "w");
    file_write(string_to_view(&contents), ir_file);
    file_close(ir_file);
    string_destroy(&contents);
    string_destroy(&ir_path);
}

void context_create_assembly_artifact(Context *restrict context) {
    x86_Context  x86_context = x86_context_create(context);
    SymbolTable *table       = &context->global_symbol_table;

    for (u64 index = 0; index < table->capacity; ++index) {
        Symbol *element = table->elements[index];
        if (element == NULL) { continue; }
        x86_codegen_symbol(element, &x86_context);
    }

    x86_emit(&x86_context);
    x86_context_destroy(&x86_context);
}

void context_create_object_artifact(Context *restrict context);
void context_create_executable_artifact(Context *restrict context);
void context_cleanup_ir_artifact(Context *restrict context);
void context_cleanup_assembly_artifact(Context *restrict context);
void context_cleanup_object_artifact(Context *restrict context);

StringView context_source_path(Context const *restrict context) {
    assert(context != nullptr);
    return string_to_view(&(context->source_path));
}

StringView context_ir_path(Context const *context) {
    assert(context != nullptr);
    return string_to_view(&(context->ir_path));
}

StringView context_assembly_path(Context const *context) {
    assert(context != nullptr);
    return string_to_view(&context->assembly_path);
}

StringView context_object_path(Context const *context) {
    assert(context != nullptr);
    return string_to_view(&context->object_path);
}

StringView context_executable_path(Context const *context) {
    assert(context != nullptr);
    return string_to_view(&(context->executable_path));
}

void context_print_error(Context const *restrict context,
                         StringView file,
                         u64        line) {
    assert(context != NULL);
    error_print(&context->current_error, file, line);
}

bool context_failure(Context *restrict context,
                     ErrorCode  code,
                     StringView message) {
    error_assign(&context->current_error, code, message);
    return false;
}

bool context_failure_string(Context *restrict context,
                            ErrorCode code,
                            String    string) {
    error_assign_string(&context->current_error, code, string);
    return false;
}

bool context_failure_type_is_not_callable(Context *restrict context,
                                          Type const *type) {
    String buf = string_create();
    string_append(&buf, SV("Type is not callable ["));
    print_type(&buf, type);
    string_append(&buf, SV("]"));
    return context_failure_string(context, ERROR_ANALYSIS_TYPE_MISMATCH, buf);
}

bool context_failure_type_is_not_indexable(Context *restrict context,
                                           Type const *type) {
    String buf = string_create();
    string_append(&buf, SV("Type is not indexable ["));
    print_type(&buf, type);
    string_append(&buf, SV("]"));
    return context_failure_string(context, ERROR_ANALYSIS_TYPE_MISMATCH, buf);
}

bool context_failure_operand_is_not_an_index(Context *restrict context,
                                             Operand operand) {
    String buf = string_create();
    string_append(&buf, SV("Operand ["));
    print_operand(&buf, operand, context);
    string_append(&buf, SV("]"));
    return context_failure_string(
        context, ERROR_ANALYSIS_OPERAND_IS_NOT_AN_INDEX, buf);
}

bool context_failure_index_out_of_bounds(Context *restrict context,
                                         u64 bound,
                                         u64 index) {
    String buf = string_create();
    string_append(&buf, SV("Index ["));
    string_append_u64(&buf, index);
    string_append(&buf, SV("] out of range [0.."));
    string_append_u64(&buf, bound);
    string_append(&buf, SV("]"));
    return context_failure_string(
        context, ERROR_ANALYSIS_INDEX_OUT_OF_BOUNDS, buf);
}

bool context_failure_mismatch_argument_count(Context *restrict context,
                                             u8 expected,
                                             u8 actual) {
    String buf = string_create();
    string_append(&buf, SV("Expected "));
    string_append_u64(&buf, expected);
    string_append(&buf, SV(" arguments, have "));
    string_append_u64(&buf, actual);
    return context_failure_string(context, ERROR_ANALYSIS_TYPE_MISMATCH, buf);
}

bool context_failure_mismatch_type(Context *restrict context,
                                   Type const *expected,
                                   Type const *actual) {
    String buf = string_create();
    string_append(&buf, SV("Expected ["));
    print_type(&buf, expected);
    string_append(&buf, SV("] Actual ["));
    print_type(&buf, actual);
    string_append(&buf, SV("]"));
    return context_failure_string(context, ERROR_ANALYSIS_TYPE_MISMATCH, buf);
}

bool context_failure_unsupported_operand(Context *restrict context,
                                         StringView operator,
                                         Operand operand) {
    String buffer = string_create();
    string_append(&buffer, SV("Operand ["));
    print_operand(&buffer, operand, context);
    string_append(&buffer, SV("] with Type ["));
    Frame *frame = context_frames_top(context);
    print_type(&buffer, type_of_operand(operand, frame->function, context));
    string_append(&buffer, SV("] is unsupported for operator ["));
    string_append(&buffer, operator);
    string_append(&buffer, SV("]"));
    return context_failure_string(
        context, ERROR_ANALYSIS_UNSUPPORTED_OPERAND, buffer);
}

bool context_failure_unsupported_operand_value(Context *restrict context,
                                               StringView operator,
                                               Value const * restrict value) {
    String buffer = string_create();
    string_append(&buffer, SV("Operand ["));
    print_value(&buffer, value, context);
    string_append(&buffer, SV("] with Type ["));
    Frame *frame = context_frames_top(context);
    print_type(&buffer, type_of_value(value, frame->function, context));
    string_append(&buffer, SV("] is unsupported for operator ["));
    string_append(&buffer, operator);
    string_append(&buffer, SV("]"));
    return context_failure_string(
        context, ERROR_ANALYSIS_UNSUPPORTED_OPERAND, buffer);
}

bool context_failure_undefined_symbol(Context *restrict context,
                                      StringView symbol) {
    String string;
    string_initialize(&string);
    string_append(&string, symbol);
    return context_failure_string(
        context, ERROR_ANALYSIS_UNDEFINED_SYMBOL, string);
}

bool context_failure_unsigned_overflow(Context *restrict context,
                                       StringView operator,
                                       Type const * type,
                                       u64 A,
                                       u64 B) {
    String buffer = string_create();
    string_append(&buffer, SV("Expression ["));
    string_append_u64(&buffer, A);
    string_append(&buffer, operator);
    string_append_u64(&buffer, B);
    string_append(&buffer, SV("] Underlying Type ["));
    print_type(&buffer, type);
    string_append(&buffer, SV("]"));
    return context_failure_string(
        context, ERROR_EVALUATION_UNSIGNED_OVERFLOW, buffer);
}

bool context_failure_signed_overflow(Context *restrict context,
                                     StringView operator,
                                     Type const * type,
                                     i64 A,
                                     i64 B) {
    String buffer = string_create();
    string_append(&buffer, SV("Expression ["));
    string_append_i64(&buffer, A);
    string_append(&buffer, operator);
    string_append_i64(&buffer, B);
    string_append(&buffer, SV("] Underlying Type ["));
    print_type(&buffer, type);
    string_append(&buffer, SV("]"));
    return context_failure_string(
        context, ERROR_EVALUATION_SIGNED_OVERFLOW, buffer);
}

ConstantString *context_intern(Context *context, StringView sv) {
    assert(context != nullptr);
    return string_interner_insert(&(context->string_interner), sv);
}

Type const *context_nil_type(Context *context) {
    assert(context != nullptr);
    return type_interner_nil_type(&(context->type_interner));
}

Type const *context_bool_type(Context *context) {
    assert(context != nullptr);
    return type_interner_boolean_type(&(context->type_interner));
}

Type const *context_u8_type(Context *context) {
    assert(context != nullptr);
    return type_interner_u8_type(&(context->type_interner));
}

Type const *context_u16_type(Context *context) {
    assert(context != nullptr);
    return type_interner_u16_type(&(context->type_interner));
}

Type const *context_u32_type(Context *context) {
    assert(context != nullptr);
    return type_interner_u32_type(&(context->type_interner));
}

Type const *context_u64_type(Context *context) {
    assert(context != nullptr);
    return type_interner_u64_type(&(context->type_interner));
}

Type const *context_i8_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i8_type(&(context->type_interner));
}

Type const *context_i16_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i16_type(&(context->type_interner));
}

Type const *context_i32_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i32_type(&(context->type_interner));
}

Type const *context_i64_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i64_type(&(context->type_interner));
}

Type const *context_tuple_type(Context *context, TupleType tuple) {
    assert(context != nullptr);
    return type_interner_tuple_type(&context->type_interner, tuple);
}

Type const *context_function_type(Context    *context,
                                  Type const *return_type,
                                  TupleType   argument_types) {
    assert(context != nullptr);
    return type_interner_function_type(
        &context->type_interner, return_type, argument_types);
}

Value const *context_constant_nil(Context *restrict context) {
    assert(context != NULL);
    return constants_nil(&context->constants);
}

Value const *context_constant_true(Context *restrict context) {
    assert(context != NULL);
    return constants_true(&context->constants);
}

Value const *context_constant_false(Context *restrict context) {
    assert(context != NULL);
    return constants_false(&context->constants);
}

Value const *context_constant_u8(Context *restrict context, u8 u8_) {
    assert(context != NULL);
    return constants_u8(&context->constants, u8_);
}

Value const *context_constant_u16(Context *restrict context, u16 u16_) {
    assert(context != NULL);
    return constants_u16(&context->constants, u16_);
}

Value const *context_constant_u32(Context *restrict context, u32 u32_) {
    assert(context != NULL);
    return constants_u32(&context->constants, u32_);
}

Value const *context_constant_u64(Context *restrict context, u64 u64_) {
    assert(context != NULL);
    return constants_u64(&context->constants, u64_);
}

Value const *context_constant_i8(Context *restrict context, i8 i8_) {
    assert(context != NULL);
    return constants_i8(&context->constants, i8_);
}

Value const *context_constant_i16(Context *restrict context, i16 i16_) {
    assert(context != NULL);
    return constants_i16(&context->constants, i16_);
}

Value const *context_constant_i32(Context *restrict context, i32 i32_) {
    assert(context != NULL);
    return constants_i32(&context->constants, i32_);
}

Value const *context_constant_i64(Context *restrict context, i64 i64_) {
    assert(context != NULL);
    return constants_i64(&context->constants, i64_);
}

Value const *context_constant_tuple(Context *restrict context, Value *tuple) {
    assert(context != NULL);
    return constants_tuple(&context->constants, tuple);
}

Value const *context_constant_function(Context *restrict context,
                                       Value *function) {
    assert(context != NULL);
    return constants_function(&context->constants, function);
}

bool context_stack_empty(Context const *restrict context) {
    assert(context != NULL);
    return stack_empty(&context->stack);
}

u32 context_stack_size(Context const *restrict context) {
    assert(context != NULL);
    return stack_size(&context->stack);
}

void context_stack_push(Context *restrict context,
                        Value const *restrict value) {
    assert(context != NULL);
    assert(value != NULL);
    stack_push(&context->stack, value);
}

Value const *context_stack_pop(Context *restrict context) {
    assert(context != NULL);
    return stack_pop(&context->stack);
}

void context_stack_pop_n(Context *restrict context, u32 n) {
    assert(context != NULL);
    stack_pop_n(&context->stack, n);
}

Value const *
context_stack_peek(Context const *restrict context, u32 offset, u32 index) {
    assert(context != NULL);
    u32 result = 0;
    exp_assert_always(!__builtin_add_overflow(offset, index, &result));
    return stack_peek(&context->stack, result);
}

bool context_frames_empty(Context const *restrict context) {
    assert(context != NULL);
    return frames_empty(&context->frames);
}

void context_frames_push(Context *restrict context, Frame frame) {
    assert(context != NULL);
    frames_push(&context->frames, frame);
}

Frame *context_frames_top(Context const *restrict context) {
    assert(context != NULL);
    return frames_top(&context->frames);
}

void context_frames_pop(Context *restrict context) {
    assert(context != NULL);
    frames_pop(&context->frames);
}

bool context_at_top_level(Context const *restrict context) {
    assert(context != NULL);
    return context->frames.size == 1;
}

static LookupResult lookup_result_none() {
    return (LookupResult){.kind = LOOKUP_RESULT_NONE};
}

static LookupResult lookup_result_local(Local *restrict local) {
    return (LookupResult){.kind = LOOKUP_RESULT_LOCAL, .local = local};
}

static LookupResult lookup_result_global(Symbol *restrict global) {
    return (LookupResult){.kind = LOOKUP_RESULT_GLOBAL, .global = global};
}

LookupResult context_lookup_label(Context *restrict context,
                                  Function const *restrict function,
                                  StringView label) {
    exp_assert(context != NULL);
    exp_assert(function != NULL);

    if (!context_at_top_level(context)) {
        Local *local = function_lookup_local_name(function, label);
        if (local != NULL) { return lookup_result_local(local); }
    }

    Symbol *global = context_global_symbol_lookup(context, label);
    if (global->value != NULL) { return lookup_result_global(global); }

    return lookup_result_none();
}

void context_push_local_value(Context *restrict context,
                              Frame *restrict frame,
                              Local *restrict local,
                              Value const *restrict constant) {
    exp_assert(context != NULL);
    exp_assert(frame != NULL);
    exp_assert(local != NULL);
    exp_assert(constant != NULL);
    context_stack_push(context, constant);
    frame->size += 1;
    // assert that the given constant is correctly placed on the stack.
    exp_assert_debug(constant ==
                     context_stack_peek(context, frame->offset, local->ssa));
}

Symbol *context_global_symbol_lookup(Context *context, StringView name) {
    assert(context != nullptr);
    return symbol_table_at(&context->global_symbol_table, name);
}

// Value const *context_constants_at(Context *context, u32 index) {
//     assert(context != nullptr);
//     return constants_at(&(context->constants), index);
// }

// u32 context_labels_insert(Context *context, StringView symbol) {
//     assert(context != nullptr);
//     return labels_insert(&context->global_labels, symbol);
// }

// StringView context_labels_at(Context *context, u32 index) {
//     assert(context != nullptr);
//     return labels_at(&context->global_labels, index);
// }

// Function *context_enter_function(Context *c, StringView name) {
//     assert(c != nullptr);
//     Symbol *element = symbol_table_at(&c->global_symbol_table, name);
//     if (element->kind == SYMBOL_KIND_UNDEFINED) {
//         element->kind = SYMBOL_KIND_FUNCTION;
//     }

//     c->current_function = &element->function_body;
//     return c->current_function;
// }

// Function *context_current_function(Context *c) {
//     assert(c != nullptr);
//     assert(c->current_function != nullptr);
//     return c->current_function;
// }

// Bytecode *context_active_bytecode(Context *c) {
//     return &(context_current_function(c)->body);
// }

// Local *context_declare_argument(Context *c) {
//     assert(c != nullptr);
//     assert(c->current_function != nullptr);
//     return function_declare_argument(c->current_function);
// }

// Local *context_declare_local(Context *c) {
//     assert(c != nullptr);
//     assert(c->current_function != nullptr);
//     return function_declare_local(c->current_function);
// }

// Local *context_lookup_argument(Context *c, u8 index) {
//     assert(c != nullptr);
//     assert(c->current_function != nullptr);
//     return function_lookup_argument(c->current_function, index);
// }

// Local *context_lookup_local(Context *c, u32 ssa) {
//     assert(c != nullptr);
//     assert(c->current_function != nullptr);
//     return function_lookup_local(c->current_function, ssa);
// }

// Local *context_lookup_local_name(Context *c, StringView name) {
//     assert(c != nullptr);
//     assert(c->current_function != nullptr);
//     return function_lookup_local_name(c->current_function, name);
// }

// void context_leave_function(Context *c) {
//     assert(c != nullptr);
//     c->current_function = nullptr;
// }

// void context_emit_return(Context *c, Operand B) {
//     assert(c != nullptr);
//     bytecode_append(context_active_bytecode(c), instruction_return(B));
// }

// Operand context_emit_call(Context *c, Operand B, Operand C) {
//     assert(c != nullptr);
//     Operand A = operand_ssa(context_declare_local(c));
//     bytecode_append(context_active_bytecode(c), instruction_call(A, B, C));
//     return A;
// }

// Operand context_emit_dot(Context *c, Operand B, Operand C) {
//     assert(c != nullptr);
//     Operand A = operand_ssa(context_declare_local(c));
//     bytecode_append(context_active_bytecode(c), instruction_dot(A, B, C));
//     return A;
// }

// Operand context_emit_load(Context *c, Operand B) {
//     assert(c != nullptr);
//     Operand A = operand_ssa(context_declare_local(c));
//     bytecode_append(context_active_bytecode(c), instruction_let(A, B));
//     return A;
// }

// Operand context_emit_negate(Context *c, Operand B) {
//     assert(c != nullptr);
//     Operand A = operand_ssa(context_declare_local(c));
//     bytecode_append(context_active_bytecode(c), instruction_neg(A, B));
//     return A;
// }

// Operand context_emit_add(Context *c, Operand B, Operand C) {
//     assert(c != nullptr);
//     Operand A = operand_ssa(context_declare_local(c));
//     bytecode_append(context_active_bytecode(c), instruction_add(A, B, C));
//     return A;
// }

// Operand context_emit_subtract(Context *c, Operand B, Operand C) {
//     assert(c != nullptr);
//     Operand A = operand_ssa(context_declare_local(c));
//     bytecode_append(context_active_bytecode(c), instruction_sub(A, B, C));
//     return A;
// }

// Operand context_emit_multiply(Context *c, Operand B, Operand C) {
//     assert(c != nullptr);
//     Operand A = operand_ssa(context_declare_local(c));
//     bytecode_append(context_active_bytecode(c), instruction_mul(A, B, C));
//     return A;
// }

// Operand context_emit_divide(Context *c, Operand B, Operand C) {
//     assert(c != nullptr);
//     Operand A = operand_ssa(context_declare_local(c));
//     bytecode_append(context_active_bytecode(c), instruction_div(A, B, C));
//     return A;
// }

// Operand context_emit_modulus(Context *c, Operand B, Operand C) {
//     assert(c != nullptr);
//     Operand A = operand_ssa(context_declare_local(c));
//     bytecode_append(context_active_bytecode(c), instruction_mod(A, B, C));
//     return A;
// }

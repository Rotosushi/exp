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
#include "env/context_options.h"
#include "evaluate/evaluate.h"
#include "scanning/parser.h"
#include "support/assert.h"
#include "support/config.h"
#include "support/io.h"
#include "support/message.h"
#include "support/process.h"
#include "support/string.h"
#include "support/string_view.h"
#include "support/unreachable.h"

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
                    ContextOptions *restrict options) {
    assert(context != nullptr);
    assert(options != nullptr);
    context->options                = *options;
    context->options.target_context = options->target->context_allocate();
    string_initialize(&(context->source_path));
    context->current_error       = error_create();
    context->global_symbol_table = symbol_table_create();
    constants_create(&context->constants);
    stack_create(&context->stack);
    context->string_interner = string_interner_create();
    context->type_interner   = type_interner_create();
}

void context_destroy(Context *context) {
    assert(context != nullptr);
    context->options.target->context_deallocate(
        context->options.target_context);
    string_destroy(&(context->source_path));
    string_interner_destroy(&(context->string_interner));
    type_interner_destroy(&(context->type_interner));
    symbol_table_destroy(&(context->global_symbol_table));
    constants_destroy(&(context->constants));
    stack_destroy(&context->stack);
    error_destroy(&context->current_error);
}

void context_print_compile_actions(Context const *restrict context) {
    if (context_shall_create_assembly_artifact(context)) {
        status(SV("create assembly artifact"), stdout);
    }

    if (context_shall_create_object_artifact(context)) {
        status(SV("create object artifact:"), stdout);
    }

    if (context_shall_create_executable_artifact(context)) {
        status(SV("create executable artifact:"), stdout);
    }

    if (context_shall_cleanup_assembly_artifact(context)) {
        status(SV("cleanup assembly artifact"), stdout);
    }

    if (context_shall_cleanup_object_artifact(context)) {
        status(SV("cleanup object artifact"), stdout);
    }
}

bool context_shall_prolix(Context const *context) {
    assert(context != nullptr);
    return context->options.prolix;
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

bool context_shall_cleanup_assembly_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.cleanup_assembly_artifact;
}

bool context_shall_cleanup_object_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.cleanup_object_artifact;
}

void *context_get_target_context(Context const *restrict context) {
    exp_assert(context != NULL);
    return context->options.target_context;
}

i32 context_compile_source(Context *restrict context, StringView source_path) {
    exp_assert(context != NULL);
    string_assign(&context->source_path, source_path);

    String buffer;
    string_initialize(&buffer);
    file_read_all(&buffer, source_path);

    Parser parser;
    parser_create(&parser, context);
    parser_set_file(&parser, source_path);

    parser_setup(&parser, string_to_view(&buffer));
    while (!parser_done(&parser)) {
        Function expression;
        function_create(&expression);

        if (!parser_parse_expression(&parser, &expression)) {
            SourceLocation source_location;
            parser_current_source_location(&parser, &source_location);
            context_print_error(
                context, source_location.file, source_location.line);
            // #NOTE: we can also reset the parser to some valid next point in
            // the input stream, and continue parsing, hoping to catch more
            // errors than just the first one encountered. This would be handled
            // right here.
            function_destroy(&expression);
            return 1;
        }

        if (!evaluate(&expression, context)) {
            SourceLocation source_location;
            parser_current_source_location(&parser, &source_location);
            context_print_error(
                context, source_location.file, source_location.line);
            // #NOTE: we can also continue to the next iteration of the loop
            // here, hoping to signal more than just the first error encountered
            // within the input text. This would be handled here. #NOTE: the
            // source information provided here is not necessarily the most
            // accurate that it could be. #TODO: devise a method to associate
            // instructions with a source location, so error reporting can
            // create very nice and clear error messages.
            function_destroy(&expression);
            return 1;
        }

        function_destroy(&expression);
    }

    // #NOTE: at this point, the source file has been fully compiled into IR.
    // So we can move on to the creation of build artifacts.
    string_destroy(&buffer);
    return 0;
}

i32 context_create_assembly_artifact(Context *restrict context) {
    exp_assert(context != NULL);
    Target *target = context->options.target;
    String  buffer;
    string_initialize(&buffer);
    target->header(&buffer, context);

    SymbolTable *table = &context->global_symbol_table;
    for (u64 index = 0; index < table->capacity; ++index) {
        Symbol *symbol = table->elements[index];
        if (symbol == NULL) { continue; }

        target->codegen(&buffer, symbol, context);

        string_append(&buffer, SV("\n"));
    }

    target->footer(&buffer, context);

    String assembly_path;
    string_initialize(&assembly_path);
    generate_path_from_source(&assembly_path,
                              context_source_path(context),
                              target->assembly_extension);

    file_write_all(&buffer, string_to_view(&assembly_path));

    string_destroy(&buffer);
    string_destroy(&assembly_path);
    return 0;
}

i32 context_create_object_artifact(Context *restrict context) {
    exp_assert(context != NULL);
    Target *target = context->options.target;

    String assembly_path;
    string_initialize(&assembly_path);
    generate_path_from_source(&assembly_path,
                              context_source_path(context),
                              target->assembly_extension);

    String object_path;
    string_initialize(&object_path);
    generate_path_from_source(
        &object_path, context_source_path(context), target->object_extension);

    // #TODO: don't hardcode the call to the assembler. Nor the arguments given
    // to the assembler.
    char const *args[] = {
        "as",
        string_to_cstring(&assembly_path),
        "-o",
        string_to_cstring(&object_path),
        NULL,
    };

    i32 result = process("as", 4, args);

    string_destroy(&assembly_path);
    string_destroy(&object_path);
    return result;
}

i32 context_create_executable_artifact(Context *restrict context) {
    exp_assert(context != NULL);
    Target *target = context->options.target;

    String object_path;
    string_initialize(&object_path);
    generate_path_from_source(
        &object_path, context_source_path(context), target->object_extension);

    String executable_path;
    string_initialize(&executable_path);
    generate_path_from_source(&executable_path,
                              context_source_path(context),
                              target->executable_extension);

    // #TODO: place our target libraries into one of ld's standard search
    //  locations on install.
    // #TODO: figure out CPACK to create .deb files for installing/uninstalling
    //  exp from a host system. or creating SNAPs to do the same.
    // #TODO: don't hardcode our call to the linker, nor the arguments we pass
    // to it.
    char const *args[] = {
        "ld",
        "-o",
        string_to_cstring(&executable_path),
        ("-L" EXP_LIBEXP_RUNTIME_BINARY_DIR),
        "-lexp_runtime_start",
        "-lexp_runtime",
        string_to_cstring(&object_path),
        NULL,
    };

    i32 result = process("ld", 7, args);

    string_destroy(&object_path);
    string_destroy(&executable_path);

    return result;
}

void context_cleanup_assembly_artifact(Context *restrict context) {
    exp_assert(context);
    Target *target = context->options.target;

    String path;
    string_initialize(&path);
    generate_path_from_source(
        &path, context_source_path(context), target->assembly_extension);

    file_remove(string_to_cstring(&path));

    string_destroy(&path);
}

void context_cleanup_object_artifact(Context *restrict context) {
    exp_assert(context);
    Target *target = context->options.target;

    String path;
    string_initialize(&path);
    generate_path_from_source(
        &path, context_source_path(context), target->object_extension);

    file_remove(string_to_cstring(&path));

    string_destroy(&path);
}

StringView context_source_path(Context const *restrict context) {
    assert(context != nullptr);
    return string_to_view(&(context->source_path));
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
                                             u64 expected,
                                             u64 actual) {
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
    print_type(&buffer,
               context_type_of_operand(context, frame->function, operand));
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
    print_type(&buffer, context_type_of_value(context, frame->function, value));
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

Type const *context_type_of_value(Context *restrict context,
                                  Function const *restrict function,
                                  Value const *restrict value) {
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: PANIC("uninitialized Value");
    case VALUE_KIND_NIL:           return context_nil_type(context);
    case VALUE_KIND_BOOL:          return context_bool_type(context);
    case VALUE_KIND_U8:            return context_u8_type(context);
    case VALUE_KIND_U16:           return context_u16_type(context);
    case VALUE_KIND_U32:           return context_u32_type(context);
    case VALUE_KIND_U64:           return context_u64_type(context);
    case VALUE_KIND_I8:            return context_i8_type(context);
    case VALUE_KIND_I16:           return context_i16_type(context);
    case VALUE_KIND_I32:           return context_i32_type(context);
    case VALUE_KIND_I64:           return context_i64_type(context);

    case VALUE_KIND_TUPLE:
        return context_type_of_tuple(context, function, &value->tuple);

    case VALUE_KIND_FUNCTION:
        return context_type_of_function(context, &value->function);

    default: EXP_UNREACHABLE();
    }
}

Type const *context_type_of_function(Context *restrict context,
                                     Function const *restrict function) {
    exp_assert(context != NULL);
    assert(function != NULL);
    assert(function->return_type != NULL);

    TupleType argument_types = tuple_type_create();
    for (u64 i = 0; i < function->arguments.size; ++i) {
        Local      *formal_argument = function->arguments.list[i];
        Type const *argument_type   = formal_argument->type;
        tuple_type_append(&argument_types, argument_type);
    }

    return context_function_type(
        context, function->return_type, argument_types);
}

Type const *context_type_of_tuple(Context *restrict context,
                                  Function const *restrict function,
                                  Tuple const *restrict tuple) {
    exp_assert(context != NULL);
    exp_assert(function != NULL);
    exp_assert(tuple != NULL);
    TupleType tuple_type = tuple_type_create();
    for (u64 i = 0; i < tuple->length; ++i) {
        Type const *T =
            context_type_of_operand(context, function, tuple->elements[i]);
        tuple_type_append(&tuple_type, T);
    }
    return context_tuple_type(context, tuple_type);
}

Type const *context_type_of_operand(Context *restrict context,
                                    Function const *restrict function,
                                    Operand operand) {
    exp_assert(context != NULL);
    exp_assert(function != NULL);
    switch (operand.kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_lookup_local(function, operand.data.ssa);
        exp_assert_debug(local->type != NULL);
        return local->type;
    }

    case OPERAND_KIND_CONSTANT:
        return context_type_of_value(context, function, operand.data.constant);

    case OPERAND_KIND_LABEL: {
        StringView label = constant_string_to_view(operand.data.label);

        LookupResult lookup = context_lookup_label(context, function, label);
        switch (lookup.kind) {
        case LOOKUP_RESULT_NONE:   unreachable();
        case LOOKUP_RESULT_LOCAL:  return lookup.local->type;
        case LOOKUP_RESULT_GLOBAL: return lookup.global->type;
        default:                   unreachable();
        }
    }

    case OPERAND_KIND_NIL:  return context_nil_type(context);
    case OPERAND_KIND_BOOL: return context_bool_type(context);
    case OPERAND_KIND_U8:   return context_u8_type(context);
    case OPERAND_KIND_U16:  return context_u16_type(context);
    case OPERAND_KIND_U32:  return context_u32_type(context);
    case OPERAND_KIND_U64:  return context_u64_type(context);
    case OPERAND_KIND_I8:   return context_i8_type(context);
    case OPERAND_KIND_I16:  return context_i16_type(context);
    case OPERAND_KIND_I32:  return context_i32_type(context);
    case OPERAND_KIND_I64:  return context_i64_type(context);

    default: EXP_UNREACHABLE();
    }
}

u64 context_size_of(Context *restrict context, Type const *type) {
    exp_assert(context != NULL);
    exp_assert(type != NULL);
    return context->options.target->size_of(type);
}

u64 context_align_of(Context *restrict context, Type const *type) {
    exp_assert(context != NULL);
    exp_assert(type != NULL);
    return context->options.target->align_of(type);
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

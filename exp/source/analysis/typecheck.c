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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <assert.h>

#include "analysis/typecheck.h"
#include "env/error.h"
#include "intrinsics/type_of.h"
#include "utility/unreachable.h"

typedef struct subject {
    FunctionBody *function;
    Context *context;
} Subject;

static void
subject_initialize(Subject *subject, FunctionBody *function, Context *context) {
    assert(subject != nullptr);
    assert(function != nullptr);
    assert(context != nullptr);
    subject->context  = context;
    subject->function = function;
}

[[maybe_unused]] static bool validate_subject(Subject *subject) {
    if (subject == nullptr) { return false; }
    if (subject->function == nullptr) { return false; }
    if (subject->context == nullptr) { return false; }
    return true;
}

static bool error(Subject *subject, ErrorCode code, String message) {
    assert(validate_subject(subject));
    Error *current_error = context_current_error(subject->context);
    assert(current_error != nullptr);
    error_from_string(current_error, code, message);
    return false;
}

static bool error_name_undefined(Subject *subject, StringView name) {
    assert(validate_subject(subject));
    String message;
    string_initialize(&message);
    string_append(&message, SV("Name: ["));
    string_append(&message, name);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_UNDEFINED_SYMBOL, message);
}

static bool error_type_mismatch(Subject *subject,
                                Type const *expected,
                                Type const *actual) {
    assert(validate_subject(subject));
    String message;
    string_initialize(&message);
    string_append(&message, SV("Expected type: ["));
    print_type(&message, expected);
    string_append(&message, SV("] Actual type: ["));
    print_type(&message, actual);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TYPE_MISMATCH, message);
}

static bool error_type_not_callable(Subject *subject, Type const *type) {
    assert(validate_subject(subject));
    assert(type != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Type: ["));
    print_type(&message, type);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TYPE_NOT_CALLABLE, message);
}

static bool error_argument_count_mismatch(Subject *subject,
                                          TupleType const *formal,
                                          Tuple const *actual) {
    assert(validate_subject(subject));
    assert(formal != nullptr);
    assert(actual != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Expected ["));
    string_append_u64(&message, formal->count);
    string_append(&message, SV("] arguments. Have ["));
    string_append_u64(&message, actual->size);
    string_append(&message, SV("] arguments."));
    return error(subject, ERROR_TYPECHECK_TYPE_MISMATCH, message);
}

static bool error_return_type_unknown(Subject *subject) {
    assert(validate_subject(subject));
    String message;
    string_initialize(&message);
    return error(subject, ERROR_TYPECHECK_RETURN_TYPE_UNKNOWN, message);
}

static bool error_type_not_indexable(Subject *subject, Type const *type) {
    assert(validate_subject(subject));
    assert(type != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Type: ["));
    print_type(&message, type);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TYPE_NOT_INDEXABLE, message);
}

static bool error_tuple_index_not_immediate(Subject *subject) {
    assert(validate_subject(subject));
    String message;
    string_initialize(&message);
    return error(subject, ERROR_TYPECHECK_TUPLE_INDEX_NOT_IMMEDIATE, message);
}

static bool
error_tuple_index_out_of_bounds(Subject *subject, i64 index, u64 bounds) {
    assert(validate_subject(subject));
    String message;
    string_initialize(&message);
    string_append(&message, SV("Index: ["));
    string_append_i64(&message, index);
    string_append(&message, SV("] Bounds: [0.."));
    string_append_u64(&message, bounds);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TUPLE_INDEX_OUT_OF_BOUNDS, message);
}

// static bool
// typecheck_symbol(Type const **result, Symbol *symbol, Context *context);

static bool typecheck_operand(Type const **result,
                              OperandKind kind,
                              OperandData data,
                              Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_body_local_at(subject->function, data.ssa);
        assert(local != nullptr);
        // #NOTE: if we try and type a usage of an ssa local, and there is
        //  no annotated type present yet, that means we forgot to set the
        //  type annotation when we typed the declaration of the ssa local.
        //  (the ssa local in position A within any instruction) Hence the
        //  assert.
        assert(local->type != nullptr);
        *result = local->type;
        return true;
    }

    case OPERAND_KIND_CONSTANT: {
        *result =
            type_of_value(data.constant, subject->function, subject->context);
        return true;
    }

    case OPERAND_KIND_I64: {
        *result = context_i64_type(subject->context);
        return true;
    }

    case OPERAND_KIND_LABEL: {
        StringView name = constant_string_to_view(data.label);
        Local *local    = function_body_local_at_name(subject->function, name);
        if (local != nullptr) {
            if (local->type == nullptr) {
                return error_name_undefined(subject, name);
            }
            *result = local->type;
            return true;
        }

        Symbol *global   = context_symbol_table_at(subject->context, name);
        Type const *type = global->type;
        // #TODO: this will loop infinitely iff we encounter mutually recursive
        //  function calls, whose types are not annotated.
        // if (type == nullptr) {
        //    if (!typecheck_symbol(&type, global, subject->context)) {
        //        return false;
        //    }
        //}
        assert(type != nullptr);

        *result = type;
        return true;
    }

    default: EXP_UNREACHABLE();
    }
}

static Local *local_from_operand_A(Instruction I, Subject *subject) {
    assert(validate_subject(subject));
    switch (I.A_kind) {
    case OPERAND_KIND_SSA: {
        return function_body_local_at(subject->function, I.A_data.ssa);
    }

    case OPERAND_KIND_LABEL: {
        StringView name = constant_string_to_view(I.A_data.label);
        return function_body_local_at_name(subject->function, name);
    }

    default: EXP_UNREACHABLE();
    }
}

static bool
typecheck_load(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));

    Local *local = local_from_operand_A(I, subject);
    assert(local != nullptr);

    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, subject)) {
        return false;
    }
    assert(B_type != nullptr);

    local_update_type(local, B_type);
    *result = B_type;
    return true;
}

static bool
typecheck_return(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, subject)) {
        return false;
    }
    assert(B_type != nullptr);

    Type const *return_type = subject->function->return_type;
    if (return_type == nullptr) {
        subject->function->return_type = B_type;
    } else if (!type_equality(B_type, return_type)) {
        return error_type_mismatch(subject, return_type, B_type);
    }
    assert(subject->function->return_type != nullptr);

    *result = B_type;
    return true;
}

static bool
typecheck_call(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    Local *local = local_from_operand_A(I, subject);
    assert(local != nullptr);

    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, subject)) {
        return false;
    }
    assert(B_type != nullptr);
    if (B_type->kind != TYPE_KIND_FUNCTION) {
        return error_type_not_callable(subject, B_type);
    }

    FunctionType const *function_type = &B_type->function_type;
    TupleType const *formal_arguments = &function_type->argument_types;

    assert(I.C_kind == OPERAND_KIND_CONSTANT);
    Value *value = I.C_data.constant;
    assert(value->kind == VALUE_KIND_TUPLE);
    Tuple *actual_arguments = &value->tuple;

    if (formal_arguments->count != actual_arguments->size) {
        return error_argument_count_mismatch(
            subject, formal_arguments, actual_arguments);
    }

    for (u8 i = 0; i < actual_arguments->size; ++i) {
        Type const *formal_type = formal_arguments->types[i];
        Operand operand         = actual_arguments->elements[i];

        Type const *actual_type = nullptr;
        if (!typecheck_operand(
                &actual_type, operand.kind, operand.data, subject)) {
            return false;
        }
        assert(actual_type != nullptr);

        if (!type_equality(actual_type, formal_type)) {
            return error_type_mismatch(subject, formal_type, actual_type);
        }
    }

    local_update_type(local, function_type->return_type);
    *result = function_type->return_type;
    return true;
}

static bool tuple_index_out_of_bounds(i64 index, TupleType const *tuple) {
    assert(tuple != nullptr);
    return ((index < 0) || ((u64)index >= tuple->count));
}

static bool
typecheck_dot(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));

    Local *local = local_from_operand_A(I, subject);
    assert(local != nullptr);

    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, subject)) {
        return false;
    }
    assert(B_type != nullptr);
    if (B_type->kind != TYPE_KIND_TUPLE) {
        return error_type_not_indexable(subject, B_type);
    }
    TupleType const *tuple = &B_type->tuple_type;

    if (I.C_kind != OPERAND_KIND_I64) {
        return error_tuple_index_not_immediate(subject);
    }
    assert(I.C_kind == OPERAND_KIND_I64);
    i64 index = I.C_data.i64_;

    if (tuple_index_out_of_bounds(index, tuple)) {
        return error_tuple_index_out_of_bounds(subject, index, tuple->count);
    }

    local_update_type(local, tuple->types[index]);
    *result = tuple->types[index];
    return true;
}

static bool typecheck_unop(Type const **result,
                           Instruction I,
                           Type const *argument_type,
                           Type const *result_type,
                           Subject *subject) {
    assert(result != nullptr);
    assert(argument_type != nullptr);
    assert(result_type != nullptr);
    assert(validate_subject(subject));

    Local *local = local_from_operand_A(I, subject);
    assert(local != nullptr);

    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, subject)) {
        return false;
    }
    assert(B_type != nullptr);

    if (!type_equality(argument_type, B_type)) {
        return error_type_mismatch(subject, argument_type, B_type);
    }

    local_update_type(local, result_type);
    *result = result_type;
    return true;
}

static bool
typecheck_negate(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    Type const *i64_type = context_i64_type(subject->context);
    return typecheck_unop(result, I, i64_type, i64_type, subject);
}

static bool typecheck_binop(Type const **result,
                            Instruction I,
                            Type const *left_type,
                            Type const *right_type,
                            Type const *result_type,
                            Subject *subject) {
    assert(result != nullptr);
    assert(left_type != nullptr);
    assert(right_type != nullptr);
    assert(result_type != nullptr);
    assert(validate_subject(subject));

    Local *local = local_from_operand_A(I, subject);
    assert(local != nullptr);

    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, subject)) {
        return false;
    }
    assert(B_type != nullptr);

    if (!type_equality(left_type, B_type)) {
        return error_type_mismatch(subject, left_type, B_type);
    }

    Type const *C_type = nullptr;
    if (!typecheck_operand(&C_type, I.C_kind, I.C_data, subject)) {
        return false;
    }
    assert(C_type != nullptr);

    if (!type_equality(right_type, C_type)) {
        return error_type_mismatch(subject, right_type, C_type);
    }

    local_update_type(local, result_type);
    *result = result_type;
    return true;
}

static bool
typecheck_add(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    Type const *i64_type = context_i64_type(subject->context);
    return typecheck_binop(result, I, i64_type, i64_type, i64_type, subject);
}

static bool
typecheck_subtract(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    Type const *i64_type = context_i64_type(subject->context);
    return typecheck_binop(result, I, i64_type, i64_type, i64_type, subject);
}

static bool
typecheck_multiply(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    Type const *i64_type = context_i64_type(subject->context);
    return typecheck_binop(result, I, i64_type, i64_type, i64_type, subject);
}

static bool
typecheck_divide(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    Type const *i64_type = context_i64_type(subject->context);
    return typecheck_binop(result, I, i64_type, i64_type, i64_type, subject);
}

static bool
typecheck_modulus(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    Type const *i64_type = context_i64_type(subject->context);
    return typecheck_binop(result, I, i64_type, i64_type, i64_type, subject);
}

static bool
typecheck_instruction(Type const **result, Instruction I, Subject *subject) {
    assert(result != nullptr);
    assert(validate_subject(subject));
    switch (I.opcode) {
    case OPCODE_RETURN: {
        if (!typecheck_return(result, I, subject)) { return false; }
        break;
    }

    case OPCODE_CALL: {
        if (!typecheck_call(result, I, subject)) { return false; }
        break;
    }

    case OPCODE_DOT: {
        if (!typecheck_dot(result, I, subject)) { return false; }
        break;
    }

    case OPCODE_LOAD: {
        if (!typecheck_load(result, I, subject)) { return false; }
        break;
    }

    case OPCODE_NEGATE: {
        if (!typecheck_negate(result, I, subject)) { return false; }
        break;
    }

    case OPCODE_ADD: {
        if (!typecheck_add(result, I, subject)) { return false; }
        break;
    }

    case OPCODE_SUBTRACT: {
        if (!typecheck_subtract(result, I, subject)) { return false; }
        break;
    }

    case OPCODE_MULTIPLY: {
        if (!typecheck_multiply(result, I, subject)) { return false; }
        break;
    }

    case OPCODE_DIVIDE: {
        if (!typecheck_divide(result, I, subject)) { return false; }
        break;
    }

    case OPCODE_MODULUS: {
        if (!typecheck_modulus(result, I, subject)) { return false; }
        break;
    }

    default: EXP_UNREACHABLE();
    }
    return true;
}

ExpResult typecheck_function(FunctionBody *function, Context *context) {
    assert(function != nullptr);
    assert(context != nullptr);
    Subject subject;
    subject_initialize(&subject, function, context);
    Block *block = &function->block;
    for (u32 index = 0; index < block->length; ++index) {
        Type const *result = nullptr;
        if (!typecheck_instruction(&result, block->buffer[index], &subject)) {
            return EXP_FAILURE;
        }
        assert(result != nullptr);
    }

    if (function->return_type == nullptr) {
        return error_return_type_unknown(&subject);
    }

    return EXP_SUCCESS;
}

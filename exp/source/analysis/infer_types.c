/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "analysis/infer_types.h"
#include "env/error.h"
#include "imr/operand.h"
#include "imr/scalar.h"
#include "intrinsics/type_of.h"
#include "utility/assert.h"
#include "utility/result.h"
#include "utility/string.h"
#include "utility/unreachable.h"

typedef struct subject {
    Function *function;
    Context *context;
} Subject;

static void subject_initialize(Subject *subject, Function *function,
                               Context *context) {
    EXP_ASSERT(subject != nullptr);
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);
    subject->context  = context;
    subject->function = function;
}

[[maybe_unused]] static bool nonnull_subject(Subject *subject) {
    if (subject == nullptr) { return false; }
    if (subject->function == nullptr) { return false; }
    if (subject->context == nullptr) { return false; }
    return true;
}

static ExpResult error(Subject *subject, ErrorCode code, String message) {
    EXP_ASSERT(nonnull_subject(subject));
    Error *current_error = context_current_error(subject->context);
    EXP_ASSERT(current_error != nullptr);
    error_from_string(current_error, code, message);
    return EXP_FAILURE;
}

/*
static ExpResult error_name_undefined(Subject *subject, StringView name) {
    EXP_ASSERT(validate_subject(subject));
    String message;
    string_initialize(&message);
    string_append(&message, SV("Name: ["));
    string_append(&message, name);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_UNDEFINED_SYMBOL, message);
}
*/

static ExpResult error_type_mismatch(Subject *subject, Type const *expected,
                                     Type const *actual) {
    EXP_ASSERT(nonnull_subject(subject));
    String message;
    string_initialize(&message);
    string_append(&message, SV("Expected type: ["));
    print_type(&message, expected);
    string_append(&message, SV("] Actual type: ["));
    print_type(&message, actual);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TYPE_MISMATCH, message);
}

static ExpResult error_type_not_callable(Subject *subject, Type const *type) {
    EXP_ASSERT(nonnull_subject(subject));
    EXP_ASSERT(type != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Type: ["));
    print_type(&message, type);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TYPE_NOT_CALLABLE, message);
}

static ExpResult error_argument_count_mismatch(Subject *subject,
                                               TupleType const *formal,
                                               Tuple const *actual) {
    EXP_ASSERT(nonnull_subject(subject));
    EXP_ASSERT(formal != nullptr);
    EXP_ASSERT(actual != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Expected ["));
    string_append_u64(&message, formal->count);
    string_append(&message, SV("] arguments. Have ["));
    string_append_u64(&message, actual->length);
    string_append(&message, SV("] arguments."));
    return error(subject, ERROR_TYPECHECK_TYPE_MISMATCH, message);
}

static ExpResult error_return_type_unknown(Subject *subject) {
    EXP_ASSERT(nonnull_subject(subject));
    String message;
    string_initialize(&message);
    return error(subject, ERROR_TYPECHECK_RETURN_TYPE_UNKNOWN, message);
}

static ExpResult error_type_not_indexable(Subject *subject, Type const *type) {
    EXP_ASSERT(nonnull_subject(subject));
    EXP_ASSERT(type != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Type: ["));
    print_type(&message, type);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TYPE_NOT_INDEXABLE, message);
}

static ExpResult error_dot_argument_not_an_index(Subject *subject) {
    EXP_ASSERT(nonnull_subject(subject));
    String message;
    string_initialize(&message);
    return error(subject, ERROR_TYPECHECK_DOT_ARGUMENT_NOT_AN_INDEX, message);
}

static ExpResult error_tuple_index_out_of_bounds(Subject *subject, Scalar index,
                                                 u32 bounds) {
    EXP_ASSERT(scalar_is_index(index));
    EXP_ASSERT(nonnull_subject(subject));
    String message;
    string_initialize(&message);
    string_append(&message, SV("Index: ["));
    print_scalar(&message, index);
    string_append(&message, SV("] Bounds: [0.."));
    string_append_u64(&message, bounds);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TUPLE_INDEX_OUT_OF_BOUNDS, message);
}

static ExpResult error_type_not_arithmetic(Subject *subject,
                                           Type const *B_type) {
    EXP_ASSERT(nonnull_subject(subject));
    EXP_ASSERT(B_type != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Type: ["));
    print_type(&message, B_type);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TYPE_NOT_ARITHMETIC, message);
}

static ExpResult error_type_not_signed(Subject *subject, Type const *B_type) {
    EXP_ASSERT(nonnull_subject(subject));
    EXP_ASSERT(B_type != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Type: ["));
    print_type(&message, B_type);
    string_append(&message, SV("]"));
    return error(subject, ERROR_TYPECHECK_TYPE_NOT_SIGNED, message);
}

// static bool
// infer_types_symbol(Type const **result, Symbol *symbol, Context *context);

static ExpResult infer_types_operand(Type const **result, Operand operand,
                                     Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(nonnull_subject(subject));
    switch (operand.kind) {
    case OPERAND_SSA: {
        Local *local = function_local_at(subject->function, operand.data.ssa);
        EXP_ASSERT(local != nullptr);
        // #NOTE: if we try and type a usage of an ssa local, and there is
        //  no annotated type present yet, that means we forgot to set the
        //  type annotation when we typed the declaration of the ssa local.
        //  (the ssa local in position A within any instruction) Hence the
        //  EXP_ASSERT.
        EXP_ASSERT(local->type != nullptr);
        *result = local->type;
        return EXP_SUCCESS;
    }

    case OPERAND_CONSTANT: {
        Value *constant =
            context_constants_at(subject->context, operand.data.constant);
        EXP_ASSERT(constant != nullptr);
        *result = type_of_value(constant, subject->function, subject->context);
        return EXP_SUCCESS;
    }

    case OPERAND_SCALAR: {
        *result = type_of_scalar(operand.data.scalar, subject->context);
        return EXP_SUCCESS;
    }

    case OPERAND_LABEL: {
        StringView label =
            context_labels_at(subject->context, operand.data.label);
        Symbol *global   = context_symbol_table_at(subject->context, label);
        Type const *type = global->type;
        // #TODO: this will loop infinitely iff we encounter mutually recursive
        //  function calls, whose types are not annotated.
        // if (type == nullptr) {
        //    if (!typecheck_symbol(&type, global, subject->context)) {
        //        return false;
        //    }
        //}
        EXP_ASSERT(type != nullptr);

        *result = type;
        return EXP_SUCCESS;
    }

    default: EXP_UNREACHABLE();
    }
}

static Local *local_from_operand_A(Instruction *instruction, Subject *subject) {
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));
    switch (instruction->A.kind) {
    case OPERAND_SSA: {
        return function_local_at(subject->function, instruction->A.data.ssa);
    }

    default: EXP_UNREACHABLE();
    }
}

static ExpResult infer_types_load(Type const **result, Instruction *instruction,
                                  Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    Local *local = local_from_operand_A(instruction, subject);
    EXP_ASSERT(local != nullptr);

    Type const *B_type = nullptr;
    if (infer_types_operand(&B_type, instruction->B, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(B_type != nullptr);

    local_update_type(local, B_type);
    *result = B_type;
    return true;
}

static ExpResult infer_types_return(Type const **result,
                                    Instruction *instruction,
                                    Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    Local *local = local_from_operand_A(instruction, subject);
    EXP_ASSERT(local != nullptr);

    Type const *B_type = nullptr;
    if (infer_types_operand(&B_type, instruction->B, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(B_type != nullptr);

    Type const *result_type = subject->function->return_type;
    if (result_type == nullptr) {
        subject->function->return_type = B_type;
    } else if (!type_equal(B_type, result_type)) {
        return error_type_mismatch(subject, result_type, B_type);
    }
    EXP_ASSERT(subject->function->return_type != nullptr);

    local_update_type(local, result_type);
    *result = B_type;
    return true;
}

static ExpResult infer_types_call(Type const **result, Instruction *instruction,
                                  Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));
    Local *local = local_from_operand_A(instruction, subject);
    EXP_ASSERT(local != nullptr);

    Type const *B_type = nullptr;
    if (infer_types_operand(&B_type, instruction->B, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(B_type != nullptr);
    if (B_type->kind != TYPE_FUNCTION) {
        return error_type_not_callable(subject, B_type);
    }

    FunctionType const *function_type = &B_type->function_type;
    TupleType const *formal_arguments = &function_type->argument_types;

    EXP_ASSERT(instruction->C.kind == OPERAND_CONSTANT);
    Value *value =
        context_constants_at(subject->context, instruction->C.data.constant);
    EXP_ASSERT(value->kind == VALUE_TUPLE);
    Tuple *actual_arguments = &value->tuple;

    if (formal_arguments->count != actual_arguments->length) {
        return error_argument_count_mismatch(subject, formal_arguments,
                                             actual_arguments);
    }

    for (u8 i = 0; i < actual_arguments->length; ++i) {
        Type const *formal_type = formal_arguments->types[i];
        Operand operand         = actual_arguments->elements[i];

        Type const *actual_type = nullptr;
        if (infer_types_operand(&actual_type, operand, subject) !=
            EXP_SUCCESS) {
            return EXP_FAILURE;
        }
        EXP_ASSERT(actual_type != nullptr);

        if (!type_equal(actual_type, formal_type)) {
            return error_type_mismatch(subject, formal_type, actual_type);
        }
    }

    local_update_type(local, function_type->return_type);
    *result = function_type->return_type;
    return true;
}

static bool tuple_index_out_of_bounds(Scalar index, TupleType const *tuple) {
    EXP_ASSERT(scalar_is_index(index));
    EXP_ASSERT(tuple != nullptr);
    switch (index.kind) {
    case SCALAR_I8: {
        i8 value = index.data.i8_;
        return ((value < 0) || ((u8)value >= tuple->count));
    }

    case SCALAR_I16: {
        i16 value = index.data.i16_;
        return ((value < 0) || ((u16)value >= tuple->count));
    }

    case SCALAR_I32: {
        i32 value = index.data.i32_;
        return ((value < 0) || ((u32)value >= tuple->count));
    }

    case SCALAR_I64: {
        i64 value = index.data.i64_;
        return ((value < 0) || ((u64)value >= tuple->count));
    }

    case SCALAR_U8: {
        u8 value = index.data.u8_;
        return (value >= tuple->count);
    }

    case SCALAR_U16: {
        u16 value = index.data.u16_;
        return (value >= tuple->count);
    }

    case SCALAR_U32: {
        u32 value = index.data.u32_;
        return (value >= tuple->count);
    }

    case SCALAR_U64: {
        u64 value = index.data.u64_;
        return (value >= tuple->count);
    }

    default: EXP_UNREACHABLE();
    }
}

static ExpResult infer_types_dot(Type const **result, Instruction *instruction,
                                 Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    Local *local = local_from_operand_A(instruction, subject);
    EXP_ASSERT(local != nullptr);

    Type const *B_type = nullptr;
    if (infer_types_operand(&B_type, instruction->B, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(B_type != nullptr);
    if (B_type->kind != TYPE_TUPLE) {
        return error_type_not_indexable(subject, B_type);
    }
    TupleType const *tuple = &B_type->tuple_type;

    Type const *C_type = nullptr;
    if (infer_types_operand(&C_type, instruction->C, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(C_type != nullptr);

    if (!type_is_index(C_type)) {
        return error_dot_argument_not_an_index(subject);
    }

    Scalar scalar = instruction->C.data.scalar;
    EXP_ASSERT(scalar_is_index(scalar));
    if (tuple_index_out_of_bounds(scalar, tuple)) {
        return error_tuple_index_out_of_bounds(subject, scalar, tuple->count);
    }

    Type const *result_type = tuple_type_at(tuple, scalar);
    local_update_type(local, result_type);
    *result = result_type;
    return true;
}

/**
 * @brief infers the type of the result of a unary operation.
 *
 * @note assert that the type of the operand is equal to the expected type of
 *  the unary operation. and that the result type is equal to the expected
 *  result type.
 */
static ExpResult infer_types_unop(Type const **result, Instruction *instruction,
                                  Type const *argument_type,
                                  Type const *result_type, Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(argument_type != nullptr);
    EXP_ASSERT(result_type != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    Local *local = local_from_operand_A(instruction, subject);
    EXP_ASSERT(local != nullptr);

    Type const *B_type = nullptr;
    if (infer_types_operand(&B_type, instruction->B, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(B_type != nullptr);

    if (!type_equal(argument_type, B_type)) {
        return error_type_mismatch(subject, argument_type, B_type);
    }

    if (!type_equal(result_type, B_type)) {
        return error_type_mismatch(subject, result_type, B_type);
    }

    local_update_type(local, result_type);
    *result = result_type;
    return true;
}

static ExpResult infer_types_negate(Type const **result,
                                    Instruction *instruction,
                                    Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    Type const *B_type = nullptr;
    if (infer_types_operand(&B_type, instruction->B, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(B_type != nullptr);
    if (!type_is_arithmetic(B_type)) {
        return error_type_not_arithmetic(subject, B_type);
    }
    if (!type_is_signed(B_type)) {
        return error_type_not_signed(subject, B_type);
    }

    Type const *unop_result = nullptr;
    if (infer_types_unop(&unop_result, instruction, B_type, B_type, subject) !=
        EXP_FAILURE) {
        return EXP_FAILURE;
    }

    *result = unop_result;
    return EXP_SUCCESS;
}

/**
 * @brief infers the type of the result of a binary operation.
 *
 * @note assert that the types of the operands are equal to the expected types
 *  of the binary operation. and that the result type is equal to the expected
 *  result type.
 */
static ExpResult infer_types_binop(Type const **result, Type const *left_type,
                                   Type const *right_type,
                                   Type const *result_type,
                                   Instruction *instruction, Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(left_type != nullptr);
    EXP_ASSERT(right_type != nullptr);
    EXP_ASSERT(result_type != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    Local *local = local_from_operand_A(instruction, subject);
    EXP_ASSERT(local != nullptr);

    Type const *B_type = nullptr;
    if (infer_types_operand(&B_type, instruction->B, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(B_type != nullptr);

    if (!type_equal(B_type, left_type)) {
        return error_type_mismatch(subject, left_type, B_type);
    }

    Type const *C_type = nullptr;
    if (infer_types_operand(&C_type, instruction->C, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(C_type != nullptr);

    if (!type_equal(right_type, C_type)) {
        return error_type_mismatch(subject, B_type, C_type);
    }

    if (!type_equal(result_type, B_type)) {
        return error_type_mismatch(subject, result_type, B_type);
    }

    local_update_type(local, result_type);
    *result = result_type;
    return true;
}

static ExpResult infer_types_arithmetic_binop(Type const **result,
                                              Instruction *instruction,
                                              Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));

    Type const *B_type = nullptr;
    if (infer_types_operand(&B_type, instruction->B, subject) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }
    EXP_ASSERT(B_type != nullptr);
    if (!type_is_arithmetic(B_type)) {
        return error_type_not_arithmetic(subject, B_type);
    }

    Type const *binop_result = nullptr;
    if (infer_types_binop(&binop_result, B_type, B_type, B_type, instruction,
                          subject) != EXP_FAILURE) {
        return EXP_FAILURE;
    }

    *result = binop_result;
    return EXP_SUCCESS;
}

static ExpResult infer_types_instruction(Type const **result,
                                         Instruction *instruction,
                                         Subject *subject) {
    EXP_ASSERT(result != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(nonnull_subject(subject));
    switch (instruction->opcode) {
    case OPCODE_RET:  return infer_types_return(result, instruction, subject);
    case OPCODE_CALL: return infer_types_call(result, instruction, subject);
    case OPCODE_DOT:  return infer_types_dot(result, instruction, subject);
    case OPCODE_LOAD: return infer_types_load(result, instruction, subject);
    case OPCODE_NEG:  return infer_types_negate(result, instruction, subject);
    case OPCODE_ADD:
        return infer_types_arithmetic_binop(result, instruction, subject);
    case OPCODE_SUB:
        return infer_types_arithmetic_binop(result, instruction, subject);
    case OPCODE_MUL:
        return infer_types_arithmetic_binop(result, instruction, subject);
    case OPCODE_DIV:
        return infer_types_arithmetic_binop(result, instruction, subject);
    case OPCODE_MOD:
        return infer_types_arithmetic_binop(result, instruction, subject);

    default: EXP_UNREACHABLE();
    }
}

ExpResult infer_types_of_locals(Function *function, Context *context) {
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);
    Subject subject;
    subject_initialize(&subject, function, context);
    Block *block = &function->block;
    for (u64 index = 0; index < block->length; ++index) {
        Type const *result = nullptr;
        if (infer_types_instruction(&result, block->buffer + index, &subject) !=
            EXP_SUCCESS) {
            return EXP_FAILURE;
        }
        EXP_ASSERT(result != nullptr);
    }

    if (function->return_type == nullptr) {
        return error_return_type_unknown(&subject);
    }

    return EXP_SUCCESS;
}

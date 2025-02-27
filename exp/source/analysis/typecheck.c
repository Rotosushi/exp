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
#include <stddef.h>
#include <stdlib.h>

#include "analysis/typecheck.h"
#include "env/error.h"
#include "intrinsics/type_of.h"
#include "utility/unreachable.h"

typedef struct Typechecker {
    Context *context;
    FunctionBody *function;
} Typechecker;

static void typechecker_initialize(Typechecker *typechecker, Context *context) {
    assert(typechecker != nullptr);
    typechecker->context  = context;
    typechecker->function = nullptr;
}

static bool error(Typechecker *typechecker, ErrorCode code, String message) {
    assert(typechecker != nullptr);
    assert(typechecker->context != nullptr);
    Error *current_error = context_current_error(typechecker->context);
    error_from_string(current_error, code, message);
    return false;
}

static bool error_type_mismatch(Typechecker *typechecker,
                                Type const *expected,
                                Type const *actual) {
    assert(typechecker != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Expected type: ["));
    emit_type(expected, &message);
    string_append(&message, SV("] Actual type: ["));
    emit_type(actual, &message);
    string_append(&message, SV("]"));
    return error(typechecker, ERROR_TYPECHECK_TYPE_MISMATCH, message);
}

static bool error_type_not_callable(Typechecker *typechecker,
                                    Type const *type) {
    assert(typechecker != nullptr);
    assert(type != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Type: ["));
    emit_type(type, &message);
    string_append(&message, SV("]"));
    return error(typechecker, ERROR_TYPECHECK_TYPE_NOT_CALLABLE, message);
}

static bool error_argument_count_mismatch(Typechecker *typechecker,
                                          TupleType const *formal,
                                          Tuple const *actual) {
    assert(typechecker != nullptr);
    assert(formal != nullptr);
    assert(actual != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Expected ["));
    string_append_u64(&message, formal->count);
    string_append(&message, SV("] arguments. Have ["));
    string_append_u64(&message, actual->size);
    string_append(&message, SV("] arguments."));
    return error(typechecker, ERROR_TYPECHECK_TYPE_MISMATCH, message);
}

static bool error_type_not_indexable(Typechecker *typechecker,
                                     Type const *type) {
    assert(typechecker != nullptr);
    assert(type != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Type: ["));
    emit_type(type, &message);
    string_append(&message, SV("]"));
    return error(typechecker, ERROR_TYPECHECK_TYPE_NOT_INDEXABLE, message);
}

static bool error_tuple_index_not_immediate(Typechecker *typechecker,
                                            OperandKind kind,
                                            OperandData data) {
    assert(typechecker != nullptr);
    String message;
    string_initialize(&message);
    Type const *type =
        type_of_operand(operand_construct(kind, data), typechecker->context);
    string_append(&message, SV("Index type: ["));
    emit_type(type, &message);
    string_append(&message, SV("]"));
    return error(
        typechecker, ERROR_TYPECHECK_TUPLE_INDEX_NOT_IMMEDIATE, message);
}

static bool error_tuple_index_out_of_bounds(Typechecker *typechecker,
                                            u32 index,
                                            u32 bounds) {
    assert(typechecker != nullptr);
    String message;
    string_initialize(&message);
    string_append(&message, SV("Index: ["));
    string_append_u64(&message, index);
    string_append(&message, SV("] Bounds: [0-"));
    string_append_u64(&message, bounds);
    string_append(&message, SV("]"));
    return error(
        typechecker, ERROR_TYPECHECK_TUPLE_INDEX_OUT_OF_BOUNDS, message);
}

static bool
typecheck_symbol(Type const **result, Symbol *symbol, Typechecker *typechecker);

static bool typecheck_operand(Type const **result,
                              OperandKind kind,
                              OperandData data,
                              Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    assert(typechecker->context != nullptr);
    assert(typechecker->function != nullptr);
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_body_local_at(typechecker->function, data.ssa);
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
        Value *value =
            context_constants_at(typechecker->context, data.constant);
        *result = type_of_value(value, typechecker->context);
        return true;
    }

    case OPERAND_KIND_I32: {
        *result = context_i32_type(typechecker->context);
        return true;
    }

    case OPERAND_KIND_LABEL: {
        StringView name  = context_labels_at(typechecker->context, data.label);
        Symbol *global   = context_symbol_table_at(typechecker->context, name);
        Type const *type = global->type;
        // #TODO: this will loop infinitely iff we encounter mutually recursive
        //  function calls!
        if (type == nullptr) {
            if (!typecheck_symbol(&type, global, typechecker)) { return false; }
        }

        *result = type;
        return true;
    }

    default: EXP_UNREACHABLE();
    }
}

static Local *local_from_operand_A(Instruction I, Typechecker *typechecker) {
    switch (I.A_kind) {
    case OPERAND_KIND_SSA: {
        return function_body_local_at(typechecker->function, I.A_data.ssa);
    }

    case OPERAND_KIND_LABEL: {
        StringView name =
            context_labels_at(typechecker->context, I.A_data.label);
        return function_body_local_named(typechecker->function, name);
    }

    default: EXP_UNREACHABLE();
    }
}

static bool
typecheck_load(Type const **result, Instruction I, Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);

    Local *local       = local_from_operand_A(I, typechecker);
    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, typechecker)) {
        return false;
    }
    assert(B_type != nullptr);

    local->type = B_type;
    *result     = B_type;
    return true;
}

static bool
typecheck_ret(Type const **result, Instruction I, Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    assert(typechecker->function != nullptr);
    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, typechecker)) {
        return false;
    }
    assert(B_type != nullptr);

    Type const *return_type = typechecker->function->return_type;
    if (return_type == nullptr) {
        typechecker->function->return_type = B_type;
    } else if (!type_equality(B_type, return_type)) {
        return error_type_mismatch(typechecker, return_type, B_type);
    }
    assert(typechecker->function->return_type != nullptr);

    *result = B_type;
    return true;
}

static bool
typecheck_call(Type const **result, Instruction I, Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    assert(typechecker->context != nullptr);
    assert(typechecker->function != nullptr);
    Local *local = local_from_operand_A(I, typechecker);

    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, typechecker)) {
        return false;
    }
    assert(B_type != nullptr);

    if (B_type->kind != TYPE_KIND_FUNCTION) {
        return error_type_not_callable(typechecker, B_type);
    }

    FunctionType const *function_type = &B_type->function_type;
    TupleType const *formal_arguments = &function_type->argument_types;

    assert(I.C_kind == OPERAND_KIND_CONSTANT);
    Value *value =
        context_constants_at(typechecker->context, I.C_data.constant);
    assert(value->kind == VALUE_KIND_TUPLE);
    Tuple *actual_arguments = &value->tuple;

    if (formal_arguments->count != actual_arguments->size) {
        return error_argument_count_mismatch(
            typechecker, formal_arguments, actual_arguments);
    }

    for (u8 i = 0; i < actual_arguments->size; ++i) {
        Type const *formal_type = formal_arguments->types[i];
        Operand operand         = actual_arguments->elements[i];

        Type const *actual_type = nullptr;
        if (!typecheck_operand(
                &actual_type, operand.kind, operand.data, typechecker)) {
            return false;
        }
        assert(actual_type != nullptr);

        if (!type_equality(actual_type, formal_type)) {
            return error_type_mismatch(typechecker, formal_type, actual_type);
        }
    }

    local->type = function_type->return_type;
    *result     = function_type->return_type;
    return true;
}

static bool tuple_index_out_of_bounds(i64 index, TupleType const *tuple) {
    return ((index < 0) || ((u64)index >= tuple->count));
}

static bool
typecheck_dot(Type const **result, Instruction I, Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    Local *local = local_from_operand_A(I, typechecker);

    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, typechecker)) {
        return false;
    }
    assert(B_type != nullptr);

    if (B_type->kind != TYPE_KIND_TUPLE) {
        return error_type_not_indexable(typechecker, B_type);
    }

    TupleType const *tuple = &B_type->tuple_type;

    if (I.C_kind != OPERAND_KIND_I32) {
        return error_tuple_index_not_immediate(typechecker, I.C_kind, I.C_data);
    }

    assert(I.C_kind == OPERAND_KIND_I32);
    i32 index = I.C_data.i32_;

    if (tuple_index_out_of_bounds(index, tuple)) {
        return error_tuple_index_out_of_bounds(
            typechecker, (u32)index, (u32)tuple->count);
    }

    local->type = tuple->types[index];
    *result     = tuple->types[index];
    return true;
}

static bool typecheck_unop(Type const **result,
                           Instruction I,
                           Type const *argument_type,
                           Type const *result_type,
                           Typechecker *typechecker) {
    assert(result != nullptr);
    assert(argument_type != nullptr);
    assert(result_type != nullptr);
    assert(typechecker != nullptr);

    Local *local = local_from_operand_A(I, typechecker);

    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, typechecker)) {
        return false;
    }
    assert(B_type != nullptr);

    if (!type_equality(argument_type, B_type)) {
        return error_type_mismatch(typechecker, argument_type, B_type);
    }

    local->type = result_type;
    *result     = result_type;
    return true;
}

static bool
typecheck_neg(Type const **result, Instruction I, Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    assert(typechecker->context != nullptr);
    Type const *i32_type = context_i32_type(typechecker->context);
    return typecheck_unop(result, I, i32_type, i32_type, typechecker);
}

static bool typecheck_binop(Type const **result,
                            Instruction I,
                            Type const *left_type,
                            Type const *right_type,
                            Type const *result_type,
                            Typechecker *typechecker) {
    assert(result != nullptr);
    assert(left_type != nullptr);
    assert(right_type != nullptr);
    assert(result_type != nullptr);
    assert(typechecker != nullptr);

    Local *local = local_from_operand_A(I, typechecker);

    Type const *B_type = nullptr;
    if (!typecheck_operand(&B_type, I.B_kind, I.B_data, typechecker)) {
        return false;
    }
    assert(B_type != nullptr);

    if (!type_equality(left_type, B_type)) {
        return error_type_mismatch(typechecker, left_type, B_type);
    }

    Type const *C_type = nullptr;
    if (!typecheck_operand(&C_type, I.C_kind, I.C_data, typechecker)) {
        return false;
    }
    assert(C_type != nullptr);

    if (!type_equality(right_type, C_type)) {
        return error_type_mismatch(typechecker, right_type, C_type);
    }

    local->type = result_type;
    *result     = result_type;
    return true;
}

static bool
typecheck_add(Type const **result, Instruction I, Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    Type const *i32_type = context_i32_type(typechecker->context);
    return typecheck_binop(
        result, I, i32_type, i32_type, i32_type, typechecker);
}

static bool typecheck_subtract(Type const **result,
                               Instruction I,
                               Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    Type const *i32_type = context_i32_type(typechecker->context);
    return typecheck_binop(
        result, I, i32_type, i32_type, i32_type, typechecker);
}

static bool typecheck_multiply(Type const **result,
                               Instruction I,
                               Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    Type const *i32_type = context_i32_type(typechecker->context);
    return typecheck_binop(
        result, I, i32_type, i32_type, i32_type, typechecker);
}

static bool
typecheck_divide(Type const **result, Instruction I, Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    Type const *i32_type = context_i32_type(typechecker->context);
    return typecheck_binop(
        result, I, i32_type, i32_type, i32_type, typechecker);
}

static bool typecheck_modulus(Type const **result,
                              Instruction I,
                              Typechecker *typechecker) {
    assert(result != nullptr);
    assert(typechecker != nullptr);
    Type const *i32_type = context_i32_type(typechecker->context);
    return typecheck_binop(
        result, I, i32_type, i32_type, i32_type, typechecker);
}

static bool typecheck_function(Type const **result,
                               FunctionBody *function,
                               Typechecker *typechecker) {
    assert(result != nullptr);
    assert(function != nullptr);
    assert(typechecker != nullptr);
    typechecker->function = function;
    Block *bc             = &function->block;

    Instruction *ip = bc->buffer;
    for (u32 idx = 0; idx < bc->length; ++idx) {
        Instruction I      = ip[idx];
        Type const *result = nullptr;
        switch (I.opcode) {
        case OPCODE_RETURN: {
            if (!typecheck_ret(&result, I, typechecker)) { return false; }
            break;
        }

        case OPCODE_CALL: {
            if (!typecheck_call(&result, I, typechecker)) { return false; }
            break;
        }

        case OPCODE_DOT: {
            if (!typecheck_dot(&result, I, typechecker)) { return false; }
            break;
        }

        case OPCODE_LOAD: {
            if (!typecheck_load(&result, I, typechecker)) { return false; }
            break;
        }

        case OPCODE_NEGATE: {
            if (!typecheck_neg(&result, I, typechecker)) { return false; }
            break;
        }

        case OPCODE_ADD: {
            if (!typecheck_add(&result, I, typechecker)) { return false; }
            break;
        }

        case OPCODE_SUBTRACT: {
            if (!typecheck_subtract(&result, I, typechecker)) { return false; }
            break;
        }

        case OPCODE_MULTIPLY: {
            if (!typecheck_multiply(&result, I, typechecker)) { return false; }
            break;
        }

        case OPCODE_DIVIDE: {
            if (!typecheck_divide(&result, I, typechecker)) { return false; }
            break;
        }

        case OPCODE_MODULUS: {
            if (!typecheck_modulus(&result, I, typechecker)) { return false; }
            break;
        }

        default: EXP_UNREACHABLE();
        }
        assert(result != nullptr);
    }

    *result = type_of_function(function, typechecker->context);
    return true;
}

static bool typecheck_symbol(Type const **result,
                             Symbol *symbol,
                             Typechecker *typechecker) {
    assert(result != nullptr);
    assert(symbol != nullptr);
    assert(typechecker != nullptr);

    if (symbol->type != nullptr) { return true; }

    FunctionBody *body        = &symbol->function_body;
    Type const *function_type = nullptr;
    if (!typecheck_function(&function_type, body, typechecker)) {
        return false;
    }
    assert(function_type != nullptr);

    symbol->type = function_type;
    *result      = function_type;
    return true;
}

#undef try

i32 typecheck(Context *context) {
    assert(context != nullptr);
    i32 result = EXIT_SUCCESS;
    Typechecker typechecker;
    typechecker_initialize(&typechecker, context);

    SymbolTable *symbol_table = &context->symbol_table;
    for (u64 i = 0; i < symbol_table->capacity; ++i) {
        Symbol *symbol = symbol_table->elements[i];
        if (symbol == nullptr) { continue; }
        Type const *result_type = nullptr;
        if (!typecheck_symbol(&result_type, symbol, &typechecker)) {
            Error *error = context_current_error(context);
            // #TODO: add source information
            error_print(error, context_source_path(context), 0);
            error_terminate(error);
            result |= EXIT_FAILURE;
        }
    }

    return result;
}

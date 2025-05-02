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

#include "analysis/infer_types.h"
#include "env/context.h"
#include "env/error.h"
#include "imr/type.h"
#include "intrinsics/type_of.h"
#include "support/string.h"
#include "support/unreachable.h"

static bool success(Type const **result, Type const *type) {
    *result = type;
    return true;
}

static bool
failure(Context *restrict context, ErrorCode code, StringView message) {
    error_assign(context_current_error(context), code, message);
    return false;
}

static bool
failure_string(Context *restrict context, ErrorCode code, String message) {
    error_assign_string(context_current_error(context), code, message);
    return false;
}

static bool failure_type_is_not_callable(Context *restrict context,
                                         Type const *type) {
    String buf = string_create();
    string_append(&buf, SV("Type is not callable ["));
    print_type(&buf, type);
    string_append(&buf, SV("]"));
    return failure_string(context, ERROR_ANALYSIS_TYPE_MISMATCH, buf);
}

static bool failure_type_is_not_indexable(Context *restrict context,
                                          Type const *type) {
    String buf = string_create();
    string_append(&buf, SV("Type is not indexable ["));
    print_type(&buf, type);
    string_append(&buf, SV("]"));
    return failure_string(context, ERROR_ANALYSIS_TYPE_MISMATCH, buf);
}

static bool failure_operand_is_not_index(Context *restrict context,
                                         Operand operand) {
    String buf = string_create();
    string_append(&buf, SV("Operand is not an index ["));
    print_operand(&buf, operand, context);
    string_append(&buf, SV("]"));
    return failure_string(context, ERROR_ANALYSIS_OPERAND_IS_NOT_AN_INDEX, buf);
}

static bool failure_tuple_index_out_of_bounds(Context *restrict context,
                                              u64 max,
                                              u64 index) {
    String buf = string_create();
    string_append(&buf, SV("Index ["));
    string_append_u64(&buf, index);
    string_append(&buf, SV("] out of range [0.."));
    string_append_u64(&buf, max);
    string_append(&buf, SV("]"));
    return failure_string(context, ERROR_ANALYSIS_INDEX_OUT_OF_BOUNDS, buf);
}

static bool failure_mismatch_argument_count(Context *restrict context,
                                            u64 expected,
                                            u64 actual) {
    String buf = string_create();
    string_append(&buf, SV("Expected "));
    string_append_u64(&buf, expected);
    string_append(&buf, SV(" arguments, have "));
    string_append_u64(&buf, actual);
    return failure_string(context, ERROR_ANALYSIS_TYPE_MISMATCH, buf);
}

static bool failure_mismatch_type(Context *restrict context,
                                  Type const *expected,
                                  Type const *actual) {
    String buf = string_create();
    string_append(&buf, SV("Expected ["));
    print_type(&buf, expected);
    string_append(&buf, SV("] Actual ["));
    print_type(&buf, actual);
    string_append(&buf, SV("]"));
    return failure_string(context, ERROR_ANALYSIS_TYPE_MISMATCH, buf);
}

static bool infer_types_global(Symbol *restrict element,
                               Context *restrict context);

static bool infer_types_operand(Type const **result,
                                Context *restrict context,
                                OperandKind kind,
                                OperandData data) {
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Local      *local = data.ssa;
        Type const *type  = local->type;
        if (type == NULL) {
            return failure(
                context, ERROR_ANALYSIS_UNDEFINED_SYMBOL, local->name);
        }

        return success(result, type);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *value = data.constant;
        return success(result, type_of_value(value, context));
    }

    case OPERAND_KIND_U8: {
        return success(result, context_u8_type(context));
    }

    case OPERAND_KIND_U16: {
        return success(result, context_u16_type(context));
    }

    case OPERAND_KIND_U32: {
        return success(result, context_u32_type(context));
    }

    case OPERAND_KIND_U64: {
        return success(result, context_u64_type(context));
    }

    case OPERAND_KIND_I8: {
        return success(result, context_i8_type(context));
    }

    case OPERAND_KIND_I16: {
        return success(result, context_i16_type(context));
    }

    case OPERAND_KIND_I32: {
        return success(result, context_i32_type(context));
    }

    case OPERAND_KIND_I64: {
        return success(result, context_i64_type(context));
    }

    case OPERAND_KIND_LABEL: {
        StringView  name   = constant_string_to_view(data.label);
        Symbol     *global = context_global_symbol_lookup(context, name);
        Type const *type   = global->type;
        if (type == NULL) {
            if (!infer_types_global(global, context)) { return false; }
            type = global->type;
        }

        return success(result, type);
    }

    default: EXP_UNREACHABLE();
    }
}

static bool
infer_types_let(Type const **result, Context *restrict context, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local *local = I.A_data.ssa;
    if (!infer_types_operand(&local->type, context, I.B_kind, I.B_data)) {
        return false;
    }
    return success(result, local->type);
}

static bool
infer_types_ret(Type const **result, Context *restrict context, Instruction I) {
    return infer_types_operand(result, context, I.B_kind, I.B_data);
}

static bool infer_types_call(Type const **result,
                             Context *restrict context,
                             Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local      *local = I.A_data.ssa;
    Type const *Bty;
    if (!infer_types_operand(&Bty, context, I.B_kind, I.B_data)) {
        return false;
    }

    if (!type_is_callable(Bty)) {
        return failure_type_is_not_callable(context, Bty);
    }

    FunctionType const *function_type = &Bty->function_type;
    TupleType const    *formal_types  = &function_type->argument_types;
    assert(I.C_kind == OPERAND_KIND_CONSTANT);
    Value const *value = I.C_data.constant;
    assert(value->kind == VALUE_KIND_TUPLE);
    Tuple const *actual_args = &value->tuple;

    if (formal_types->size != actual_args->size) {
        return failure_mismatch_argument_count(
            context, formal_types->size, actual_args->size);
    }

    for (u8 i = 0; i < actual_args->size; ++i) {
        Type const *formal_type = formal_types->types[i];
        Operand     operand     = actual_args->elements[i];
        Type const *actual_type;
        if (!infer_types_operand(
                &actual_type, context, operand.kind, operand.data)) {
            return false;
        }

        if (!type_equality(actual_type, formal_type)) {
            return failure_mismatch_type(context, formal_type, actual_type);
        }
    }

    local->type = function_type->return_type;
    return success(result, function_type->return_type);
}

static bool tuple_index_out_of_bounds(u64 index, TupleType const *tuple) {
    return index >= tuple->size;
}

static bool
infer_types_dot(Type const **result, Context *restrict context, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local      *local = I.A_data.ssa;
    Type const *Bty;
    if (!infer_types_operand(&Bty, context, I.B_kind, I.B_data)) {
        return false;
    }

    if (!type_is_indexable(Bty)) {
        return failure_type_is_not_indexable(context, Bty);
    }

    TupleType const *tuple = &Bty->tuple_type;
    Operand          C     = operand(I.C_kind, I.C_data);

    if (!operand_is_index(C)) {
        return failure_operand_is_not_index(context, C);
    }

    u64 index = operand_as_index(C);

    if (tuple_index_out_of_bounds(index, tuple)) {
        return failure_tuple_index_out_of_bounds(context, tuple->size, index);
    }

    local->type = tuple->types[index];
    return success(result, tuple->types[index]);
}

static bool infer_types_unop(Type const **result,
                             Context *restrict c,
                             Instruction I,
                             Type const *result_type,
                             Type const *argument_type) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local *local = I.A_data.ssa;
    if (!infer_types_operand(&local->type, c, I.B_kind, I.B_data)) {
        return false;
    }

    if (!type_equality(argument_type, local->type)) {
        return failure_mismatch_type(c, argument_type, local->type);
    }

    return success(result, result_type);
}

static bool
infer_types_neg(Type const **result, Context *restrict c, Instruction I) {
    Type const *type_i64 = context_i64_type(c);
    return infer_types_unop(result, c, I, type_i64, type_i64);
}

static bool infer_types_binop(Type const **result,
                              Context *restrict c,
                              Instruction I,
                              Type const *result_type,
                              Type const *lhs_type,
                              Type const *rhs_type) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    Local      *local = I.A_data.ssa;
    Type const *Bty;
    if (!infer_types_operand(&Bty, c, I.B_kind, I.B_data)) { return false; }
    if (!type_equality(lhs_type, Bty)) {
        return failure_mismatch_type(c, lhs_type, Bty);
    }
    Type const *Cty;
    if (!infer_types_operand(&Cty, c, I.C_kind, I.C_data)) { return false; }
    if (!type_equality(rhs_type, Cty)) {
        return failure_mismatch_type(c, rhs_type, Cty);
    }
    local->type = result_type;
    return success(result, result_type);
}

static bool
infer_types_add(Type const **result, Context *restrict c, Instruction I) {
    Type const *type_i64 = context_i64_type(c);
    return infer_types_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool
infer_types_sub(Type const **result, Context *restrict c, Instruction I) {
    Type const *type_i64 = context_i64_type(c);
    return infer_types_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool
infer_types_mul(Type const **result, Context *restrict c, Instruction I) {
    Type const *type_i64 = context_i64_type(c);
    return infer_types_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool
infer_types_div(Type const **result, Context *restrict c, Instruction I) {
    Type const *type_i64 = context_i64_type(c);
    return infer_types_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool
infer_types_mod(Type const **result, Context *restrict c, Instruction I) {
    Type const *type_i64 = context_i64_type(c);
    return infer_types_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool infer_types_function(Type const **restrict result,
                                 Function *restrict function,
                                 Context *restrict context) {
    Bytecode *body = &function->body;

    Instruction *ip = body->buffer;
    for (u32 idx = 0; idx < body->length; ++idx) {
        Instruction I = ip[idx];
        switch (I.opcode) {
        case OPCODE_RET: {
            Type const *Bty;
            if (!infer_types_ret(&Bty, context, I)) { return false; }

            if ((function->return_type != NULL) &&
                (!type_equality(function->return_type, Bty))) {
                return failure_mismatch_type(
                    context, function->return_type, Bty);
            }

            function->return_type = Bty;
            break;
        }

        case OPCODE_CALL: {
            Type const *Aty;
            if (!infer_types_call(&Aty, context, I)) { return false; }
            break;
        }

        case OPCODE_LET: {
            Type const *Aty;
            if (!infer_types_let(&Aty, context, I)) { return false; }
            break;
        }

        case OPCODE_DOT: {
            Type const *Aty;
            if (!infer_types_dot(&Aty, context, I)) { return false; }
            break;
        }

        case OPCODE_NEG: {
            Type const *Aty;
            if (!infer_types_neg(&Aty, context, I)) { return false; }
            break;
        }

        case OPCODE_ADD: {
            Type const *Aty;
            if (!infer_types_add(&Aty, context, I)) { return false; }
            break;
        }

        case OPCODE_SUB: {
            Type const *Aty;
            if (!infer_types_sub(&Aty, context, I)) { return false; }
            break;
        }

        case OPCODE_MUL: {
            Type const *Aty;
            if (!infer_types_mul(&Aty, context, I)) { return false; }
            break;
        }

        case OPCODE_DIV: {
            Type const *Aty;
            if (!infer_types_div(&Aty, context, I)) { return false; }
            break;
        }

        case OPCODE_MOD: {
            Type const *Aty;
            if (!infer_types_mod(&Aty, context, I)) { return false; }
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }

    return success(result, type_of_function(function, context));
}

static bool infer_types_global(Symbol *restrict element,
                               Context *restrict context) {
    assert(element->value->kind == VALUE_KIND_FUNCTION);
    Type const *result = NULL;
    if (!infer_types_function(
            &result, (Function *)&element->value->function, context)) {
        return false;
    }
    return true;
}

bool infer_types(Function *restrict function, Context *restrict context) {
    Type const *result = NULL;
    return infer_types_function(&result, function, context);
}

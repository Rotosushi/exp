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
#include <stddef.h>
#include <stdlib.h>

#include "analysis/infer_types.h"
#include "env/context.h"
#include "imr/type.h"
#include "support/assert.h"
#include "support/unreachable.h"

static bool success(Type const **result, Type const *type) {
    *result = type;
    return true;
}

static bool infer_types_operand(Type const **result,
                                Function *restrict function,
                                Context *restrict context,
                                OperandKind kind,
                                OperandData data) {
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Local      *local = function_lookup_local(function, data.ssa);
        Type const *type  = local->type;
        // #NOTE: since we are looking up a local, already defined,
        // by definition we must have inferred the type of it already.
        // the let instruction must come before this usage of the name.
        // therefore if the type is not filled in, this is a bug in
        // our implementation
        exp_assert_debug(type != NULL);
        return success(result, type);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *value = data.constant;
        return success(result, context_type_of_value(context, function, value));
    }

    case OPERAND_KIND_LABEL: {
        StringView   label  = constant_string_to_view(data.label);
        LookupResult lookup = context_lookup_label(context, function, label);

        Type const *type = NULL;
        switch (lookup.kind) {
        case LOOKUP_RESULT_NONE:
            return context_failure_undefined_symbol(context, label);
        case LOOKUP_RESULT_LOCAL: {
            type = lookup.local->type;
            break;
        }
        case LOOKUP_RESULT_GLOBAL: {
            type = lookup.global->type;
            break;
        }
        }
        exp_assert_debug(type != NULL);

        return success(result, type);
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

    default: EXP_UNREACHABLE();
    }
}

static bool infer_types_let(Type const **result,
                            Function *restrict function,
                            Context *restrict context,
                            Instruction I) {
    exp_assert_debug(I.A_kind == OPERAND_KIND_SSA);
    Local *local = function_lookup_local(function, I.A_data.ssa);
    if (!infer_types_operand(
            &local->type, function, context, I.B_kind, I.B_data)) {
        return false;
    }
    return success(result, local->type);
}

static bool infer_types_ret(Type const **result,
                            Function *restrict function,
                            Context *restrict context,
                            Instruction I) {
    return infer_types_operand(result, function, context, I.B_kind, I.B_data);
}

static bool infer_types_call(Type const **result,
                             Function *restrict function,
                             Context *restrict context,
                             Instruction I) {
    exp_assert_debug(I.A_kind == OPERAND_KIND_SSA);
    Local      *local = function_lookup_local(function, I.A_data.ssa);
    Type const *Bty;
    if (!infer_types_operand(&Bty, function, context, I.B_kind, I.B_data)) {
        return false;
    }

    if (!type_is_callable(Bty)) {
        return context_failure_type_is_not_callable(context, Bty);
    }

    FunctionType const *function_type = &Bty->function_type;
    TupleType const    *formal_types  = &function_type->argument_types;
    exp_assert_debug(I.C_kind == OPERAND_KIND_CONSTANT);
    Value const *value = I.C_data.constant;
    exp_assert_debug(value->kind == VALUE_KIND_TUPLE);
    Tuple const *actual_args = &value->tuple;

    if (formal_types->size != actual_args->length) {
        return context_failure_mismatch_argument_count(
            context, formal_types->size, actual_args->length);
    }

    for (u8 i = 0; i < actual_args->length; ++i) {
        Type const *formal_type = formal_types->types[i];
        Operand     operand     = actual_args->elements[i];
        Type const *actual_type;
        if (!infer_types_operand(
                &actual_type, function, context, operand.kind, operand.data)) {
            return false;
        }

        if (!type_equality(actual_type, formal_type)) {
            return context_failure_mismatch_type(
                context, formal_type, actual_type);
        }
    }

    local->type = function_type->return_type;
    return success(result, function_type->return_type);
}

static bool infer_types_dot(Type const **result,
                            Function *restrict function,
                            Context *restrict context,
                            Instruction I) {
    exp_assert(I.A_kind == OPERAND_KIND_SSA);
    Local      *local = function_lookup_local(function, I.A_data.ssa);
    Type const *Bty;
    if (!infer_types_operand(&Bty, function, context, I.B_kind, I.B_data)) {
        return false;
    }

    if (!type_is_indexable(Bty)) {
        return context_failure_type_is_not_indexable(context, Bty);
    }

    TupleType const *tuple = &Bty->tuple_type;
    Operand          C     = operand(I.C_kind, I.C_data);

    if (!operand_is_index(C)) {
        return context_failure_operand_is_not_an_index(context, C);
    }

    u64 index = operand_as_index(C);
    exp_assert(index < u32_MAX);
    if (!tuple_type_index_in_bounds(tuple, (u32)index)) {
        return context_failure_index_out_of_bounds(context, tuple->size, index);
    }

    local->type = tuple->types[index];
    return success(result, tuple->types[index]);
}

static bool infer_types_unop(Type const **result,
                             Function *restrict function,
                             Context *restrict context,
                             Instruction I,
                             Type const *result_type,
                             Type const *argument_type) {
    exp_assert(I.A_kind == OPERAND_KIND_SSA);
    Local *local = function_lookup_local(function, I.A_data.ssa);
    if (!infer_types_operand(
            &local->type, function, context, I.B_kind, I.B_data)) {
        return false;
    }

    if (!type_equality(argument_type, local->type)) {
        return context_failure_mismatch_type(
            context, argument_type, local->type);
    }

    return success(result, result_type);
}

static bool infer_types_neg(Type const **result,
                            Function *restrict function,
                            Context *restrict context,
                            Instruction I) {
    Type const *type_i64 = context_i64_type(context);
    return infer_types_unop(result, function, context, I, type_i64, type_i64);
}

static bool infer_types_binop(Type const **result,
                              Function *restrict function,
                              Context *restrict context,
                              Instruction I,
                              Type const *result_type,
                              Type const *lhs_type,
                              Type const *rhs_type) {
    // #TODO: Integer promotion rules
    exp_assert(I.A_kind == OPERAND_KIND_SSA);
    Local      *local = function_lookup_local(function, I.A_data.ssa);
    Type const *Bty;
    if (!infer_types_operand(&Bty, function, context, I.B_kind, I.B_data)) {
        return false;
    }
    if (!type_equality(lhs_type, Bty)) {
        return context_failure_mismatch_type(context, lhs_type, Bty);
    }
    Type const *Cty;
    if (!infer_types_operand(&Cty, function, context, I.C_kind, I.C_data)) {
        return false;
    }
    if (!type_equality(rhs_type, Cty)) {
        return context_failure_mismatch_type(context, rhs_type, Cty);
    }
    local->type = result_type;
    return success(result, result_type);
}

static bool infer_types_add(Type const **result,
                            Function *restrict function,
                            Context *restrict context,
                            Instruction I) {
    Type const *type_i64 = context_i64_type(context);
    return infer_types_binop(
        result, function, context, I, type_i64, type_i64, type_i64);
}

static bool infer_types_sub(Type const **result,
                            Function *restrict function,
                            Context *restrict c,
                            Instruction I) {
    Type const *type_i64 = context_i64_type(c);
    return infer_types_binop(
        result, function, c, I, type_i64, type_i64, type_i64);
}

static bool infer_types_mul(Type const **result,
                            Function *restrict function,
                            Context *restrict context,
                            Instruction I) {
    Type const *type_i64 = context_i64_type(context);
    return infer_types_binop(
        result, function, context, I, type_i64, type_i64, type_i64);
}

static bool infer_types_div(Type const **result,
                            Function *restrict function,
                            Context *restrict context,
                            Instruction I) {
    Type const *type_i64 = context_i64_type(context);
    return infer_types_binop(
        result, function, context, I, type_i64, type_i64, type_i64);
}

static bool infer_types_mod(Type const **result,
                            Function *restrict function,
                            Context *restrict context,
                            Instruction I) {
    Type const *type_i64 = context_i64_type(context);
    return infer_types_binop(
        result, function, context, I, type_i64, type_i64, type_i64);
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
            if (!infer_types_ret(&Bty, function, context, I)) { return false; }

            if ((function->return_type != NULL) &&
                (!type_equality(function->return_type, Bty))) {
                return context_failure_mismatch_type(
                    context, function->return_type, Bty);
            }

            function->return_type = Bty;
            break;
        }

        case OPCODE_CALL: {
            Type const *Aty;
            if (!infer_types_call(&Aty, function, context, I)) { return false; }
            break;
        }

        case OPCODE_LET: {
            Type const *Aty;
            if (!infer_types_let(&Aty, function, context, I)) { return false; }
            break;
        }

        case OPCODE_NEG: {
            Type const *Aty;
            if (!infer_types_neg(&Aty, function, context, I)) { return false; }
            break;
        }

        case OPCODE_DOT: {
            Type const *Aty;
            if (!infer_types_dot(&Aty, function, context, I)) { return false; }
            break;
        }

        case OPCODE_ADD: {
            Type const *Aty;
            if (!infer_types_add(&Aty, function, context, I)) { return false; }
            break;
        }

        case OPCODE_SUB: {
            Type const *Aty;
            if (!infer_types_sub(&Aty, function, context, I)) { return false; }
            break;
        }

        case OPCODE_MUL: {
            Type const *Aty;
            if (!infer_types_mul(&Aty, function, context, I)) { return false; }
            break;
        }

        case OPCODE_DIV: {
            Type const *Aty;
            if (!infer_types_div(&Aty, function, context, I)) { return false; }
            break;
        }

        case OPCODE_MOD: {
            Type const *Aty;
            if (!infer_types_mod(&Aty, function, context, I)) { return false; }
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }

    return success(result, context_type_of_function(context, function));
}

bool infer_types(Function *restrict function, Context *restrict context) {
    Type const *result = NULL;
    return infer_types_function(&result, function, context);
}

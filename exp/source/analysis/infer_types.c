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
#include "imr/type/composite.h"
#include "imr/value.h"
#include "support/assert.h"
#include "support/unreachable.h"

static bool success(Type const **result, Type const *type) {
    *result = type;
    return true;
}

static bool infer_types_function(Type const **restrict result,
                                 Function *restrict function,
                                 Context *restrict context);

static bool infer_types_operand(Type const **result,
                                Function *restrict function,
                                Context *restrict context,
                                OperandKind kind,
                                OperandData data);

static bool infer_types_constant(Type const **result,
                                 Function *restrict function,
                                 Context *restrict context,
                                 Value const *restrict constant) {
    switch (constant->kind) {
    case VALUE_KIND_UNINITIALIZED: {
        return context_failure_uninitialized_value(context);
    }

    case VALUE_KIND_NIL:  return success(result, context_nil_type(context));
    case VALUE_KIND_BOOL: return success(result, context_bool_type(context));
    case VALUE_KIND_U8:   return success(result, context_u8_type(context));
    case VALUE_KIND_U16:  return success(result, context_u16_type(context));
    case VALUE_KIND_U32:  return success(result, context_u32_type(context));
    case VALUE_KIND_U64:  return success(result, context_u64_type(context));
    case VALUE_KIND_I8:   return success(result, context_i8_type(context));
    case VALUE_KIND_I16:  return success(result, context_i16_type(context));
    case VALUE_KIND_I32:  return success(result, context_i32_type(context));
    case VALUE_KIND_I64:  return success(result, context_i64_type(context));

    case VALUE_KIND_TUPLE: {
        Tuple const *tuple = &constant->tuple;
        TypeTuple    tuple_type;
        type_tuple_create(&tuple_type);
        for (u32 index = 0; index < tuple->length; ++index) {
            Operand     element      = tuple->elements[index];
            Type const *element_type = NULL;
            if (!infer_types_operand(&element_type,
                                     function,
                                     context,
                                     element.kind,
                                     element.data)) {
                return false;
            }
            type_tuple_append(&tuple_type, element_type);
        }
        return success(result, context_tuple_type(context, tuple_type));
    }

    case VALUE_KIND_FUNCTION: {
        return infer_types_function(
            result, (Function *)&constant->function, context);
    }

    default: EXP_UNREACHABLE();
    }
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
        return infer_types_constant(result, function, context, data.constant);
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

static bool verify_argument(Type const *formal,
                            Type const *actual,
                            Context *restrict context) {
    switch (formal->kind) {
    case TYPE_KIND_PRIMARY: {
        if (!type_equality(formal, actual)) {
            return context_failure_mismatch_type(context, formal, actual);
        }
        break;
    }

    case TYPE_KIND_COMPOSITE: {
        if (actual->kind != TYPE_KIND_COMPOSITE) {
            return context_failure_mismatch_type(context, formal, actual);
        }

        switch (formal->composite.kind) {
        case TYPE_COMPOSITE_KIND_TUPLE: {
            if (actual->composite.kind != TYPE_COMPOSITE_KIND_TUPLE) {
                return context_failure_mismatch_type(context, formal, actual);
            }

            TypeTuple const *formal_tuple = &formal->composite.data.tuple;
            TypeTuple const *actual_tuple = &actual->composite.data.tuple;

            if (formal_tuple->length != actual_tuple->length) {
                return context_failure_mismatch_argument_count(
                    context, formal_tuple->length, actual_tuple->length);
            }

            for (u32 index = 0; index < formal_tuple->length; ++index) {
                Type const *formal_argument = formal_tuple->types[index];
                Type const *actual_argument = actual_tuple->types[index];

                if (!type_equality(formal_argument, actual_argument)) {
                    return context_failure_mismatch_type(
                        context, formal_argument, actual_argument);
                }

                break;
            }

            break;
        }

        case TYPE_COMPOSITE_KIND_FUNCTION: {
            if (actual->composite.kind != TYPE_COMPOSITE_KIND_FUNCTION) {
                return context_failure_mismatch_type(context, formal, actual);
            }

            TypeFunction const *formal_function =
                &formal->composite.data.function;
            TypeFunction const *actual_function =
                &actual->composite.data.function;

            Type const *formal_result = formal_function->result;
            Type const *actual_result = actual_function->result;

            if (!type_equality(formal_result, actual_result)) {
                return context_failure_mismatch_type(
                    context, formal_result, actual_result);
            }

            Type const *formal_argument = formal_function->argument;
            Type const *actual_argument = actual_function->argument;

            if (!type_equality(formal_argument, actual_argument)) {
                return context_failure_mismatch_type(
                    context, formal_argument, actual_argument);
            }

            break;
        }

        default: EXP_UNREACHABLE();
        }
    }
    }

    return true;
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

    exp_assert_debug(Bty->kind == TYPE_KIND_COMPOSITE);
    exp_assert_debug(Bty->composite.kind == TYPE_COMPOSITE_KIND_FUNCTION);
    TypeFunction const *function_type = &Bty->composite.data.function;
    Type const         *formal        = function_type->argument;
    exp_assert_debug(I.C_kind == OPERAND_KIND_CONSTANT);
    Value const *value  = I.C_data.constant;
    Type const  *actual = value->type;

    if (!verify_argument(formal, actual, context)) { return false; }

    local->type = function_type->result;
    return success(result, function_type->result);
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

    exp_assert_debug(Bty->kind == TYPE_KIND_COMPOSITE);
    exp_assert_debug(Bty->composite.kind == TYPE_COMPOSITE_KIND_TUPLE);
    TypeTuple const *tuple = &Bty->composite.data.tuple;
    Operand          C     = operand(I.C_kind, I.C_data);

    if (!operand_is_index(C)) {
        return context_failure_operand_is_not_an_index(context, C);
    }

    u64 index = operand_as_index(C);
    exp_assert(index < u32_MAX);
    if (!type_tuple_index_in_bounds(tuple, (u32)index)) {
        return context_failure_index_out_of_bounds(
            context, tuple->length, index);
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
    Type const  *return_type = NULL;
    Block       *body        = &function->body;
    Instruction *ip          = body->buffer;
    for (u32 idx = 0; idx < body->length; ++idx) {
        Instruction I = ip[idx];
        switch (I.opcode) {
        case OPCODE_RET: {
            if (!infer_types_ret(&return_type, function, context, I)) {
                return false;
            }

            if ((function->result->type != NULL) &&
                (!type_equality(function->result->type, return_type))) {
                return context_failure_mismatch_type(
                    context, function->result->type, return_type);
            }

            function->result->type = return_type;
            break;
        }

        case OPCODE_CALL: {
            if (!infer_types_call(&return_type, function, context, I)) {
                return false;
            }
            break;
        }

        case OPCODE_LET: {
            if (!infer_types_let(&return_type, function, context, I)) {
                return false;
            }
            break;
        }

        case OPCODE_NEG: {
            if (!infer_types_neg(&return_type, function, context, I)) {
                return false;
            }
            break;
        }

        case OPCODE_DOT: {
            if (!infer_types_dot(&return_type, function, context, I)) {
                return false;
            }
            break;
        }

        case OPCODE_ADD: {
            if (!infer_types_add(&return_type, function, context, I)) {
                return false;
            }
            break;
        }

        case OPCODE_SUB: {
            if (!infer_types_sub(&return_type, function, context, I)) {
                return false;
            }
            break;
        }

        case OPCODE_MUL: {
            if (!infer_types_mul(&return_type, function, context, I)) {
                return false;
            }
            break;
        }

        case OPCODE_DIV: {
            if (!infer_types_div(&return_type, function, context, I)) {
                return false;
            }
            break;
        }

        case OPCODE_MOD: {
            if (!infer_types_mod(&return_type, function, context, I)) {
                return false;
            }
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }

    // #NOTE: We are not accounting for the fact that a top level expression
    // is not required to have a return statement. And since we expect each
    // function to use their required return statement to give us something to
    // use for the return type, we assert here. We can solve this by letting the
    // last statement in a block implicitly be a return statement. But we have
    // not accounted for that anywhere else.

    // #NOTE: first, an empty function body is an implicit `return nil;`
    if (return_type == NULL) { return_type = context_nil_type(context); }

    // #NOTE: second, if the function did not have it's return type set by a
    // return instruction, then we use the type of it's last instruction as
    // it's return type. return_type will hold this value after we finish
    // iterating through all of the instructions as a matter of course.
    if (function->result->type == NULL) {
        function->result->type = return_type;
    }

    return success(result, context_type_of_function(context, function));
}

bool infer_types(Function *restrict function, Context *restrict context) {
    Type const *result = NULL;
    return infer_types_function(&result, function, context);
}

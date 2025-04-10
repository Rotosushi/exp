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
#include "env/context.h"
#include "env/error.h"
#include "imr/type.h"
#include "intrinsics/type_of.h"
#include "support/message.h"
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
    return failure_string(context, ERROR_TYPECHECK_TYPE_MISMATCH, buf);
}

static bool failure_type_is_not_indexable(Context *restrict context,
                                          Type const *type) {
    String buf = string_create();
    string_append(&buf, SV("Type is not indexable ["));
    print_type(&buf, type);
    string_append(&buf, SV("]"));
    return failure_string(context, ERROR_TYPECHECK_TYPE_MISMATCH, buf);
}

static bool failure_operand_is_not_index(Context *restrict context,
                                         Operand operand) {
    String buf = string_create();
    string_append(&buf, SV("Operand is not an index ["));
    print_operand(&buf, operand, context);
    string_append(&buf, SV("]"));
    return failure_string(
        context, ERROR_TYPECHECK_TUPLE_INDEX_NOT_IMMEDIATE, buf);
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
    return failure_string(
        context, ERROR_TYPECHECK_TUPLE_INDEX_OUT_OF_BOUNDS, buf);
}

static bool failure_mismatch_argument_count(Context *restrict context,
                                            u64 expected,
                                            u64 actual) {
    String buf = string_create();
    string_append(&buf, SV("Expected "));
    string_append_u64(&buf, expected);
    string_append(&buf, SV(" arguments, have "));
    string_append_u64(&buf, actual);
    return failure_string(context, ERROR_TYPECHECK_TYPE_MISMATCH, buf);
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
    return failure_string(context, ERROR_TYPECHECK_TYPE_MISMATCH, buf);
}

static bool typecheck_global(Type const **result,
                             Context *restrict context,
                             Symbol *restrict element);

static bool typecheck_operand(Type const **result,
                              Context *restrict context,
                              OperandKind kind,
                              OperandData data) {
    if (context_trace(context)) { trace(SV("typecheck_operand:"), stdout); }
    switch (kind) {
    case OPERAND_KIND_SSA: {
        LocalVariable *local = context_lookup_ssa(context, data.ssa);
        Type const    *type  = local->type;
        if (type == NULL) {
            return failure(context, ERROR_TYPECHECK_UNDEFINED_SYMBOL, SV(""));
        }

        return success(result, type);
    }

    case OPERAND_KIND_CONSTANT: {
        Value *value = context_constants_at(context, data.constant);
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
        StringView  name   = context_labels_at(context, data.label);
        Symbol     *global = context_global_symbol_table_at(context, name);
        Type const *type   = global->type;
        if (type == NULL) {
            if (!typecheck_global(&type, context, global)) { return false; }
        }

        return success(result, type);
    }

    default: EXP_UNREACHABLE();
    }
}

static bool
typecheck_load(Type const **result, Context *restrict context, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    if (context_trace(context)) { trace(SV("typecheck_load:"), stdout); }
    LocalVariable *local = context_lookup_ssa(context, I.A_data.ssa);
    if (!typecheck_operand(&local->type, context, I.B_kind, I.B_data)) {
        return false;
    }
    return success(result, local->type);
}

static bool
typecheck_ret(Type const **result, Context *restrict context, Instruction I) {
    if (context_trace(context)) { trace(SV("typecheck_ret:"), stdout); }
    return typecheck_operand(result, context, I.B_kind, I.B_data);
}

static bool
typecheck_call(Type const **result, Context *restrict context, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    if (context_trace(context)) { trace(SV("typecheck_call:"), stdout); }
    LocalVariable *local = context_lookup_ssa(context, I.A_data.ssa);
    Type const    *Bty;
    if (!typecheck_operand(&Bty, context, I.B_kind, I.B_data)) { return false; }

    if (!type_is_callable(Bty)) {
        return failure_type_is_not_callable(context, Bty);
    }

    FunctionType const *function_type = &Bty->function_type;
    TupleType const    *formal_types  = &function_type->argument_types;
    assert(I.C_kind == OPERAND_KIND_CONSTANT);
    Value *value = context_constants_at(context, I.C_data.constant);
    assert(value->kind == VALUE_KIND_TUPLE);
    Tuple *actual_args = &value->tuple;

    if (formal_types->size != actual_args->size) {
        return failure_mismatch_argument_count(
            context, formal_types->size, actual_args->size);
    }

    for (u8 i = 0; i < actual_args->size; ++i) {
        Type const *formal_type = formal_types->types[i];
        Operand     operand     = actual_args->elements[i];
        Type const *actual_type;
        if (!typecheck_operand(
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
typecheck_dot(Type const **result, Context *restrict context, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    if (context_trace(context)) { trace(SV("typecheck_dot:"), stdout); }
    LocalVariable *local = context_lookup_ssa(context, I.A_data.ssa);
    Type const    *Bty;
    if (!typecheck_operand(&Bty, context, I.B_kind, I.B_data)) { return false; }

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

static bool typecheck_unop(Type const **result,
                           Context *restrict c,
                           Instruction I,
                           Type const *result_type,
                           Type const *argument_type) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = context_lookup_ssa(c, I.A_data.ssa);
    if (!typecheck_operand(&local->type, c, I.B_kind, I.B_data)) {
        return false;
    }

    if (!type_equality(argument_type, local->type)) {
        return failure_mismatch_type(c, argument_type, local->type);
    }

    return success(result, result_type);
}

static bool
typecheck_neg(Type const **result, Context *restrict c, Instruction I) {
    if (context_trace(c)) { trace(SV("typecheck_neg:"), stdout); }
    Type const *type_i64 = context_i64_type(c);
    return typecheck_unop(result, c, I, type_i64, type_i64);
}

static bool typecheck_binop(Type const **result,
                            Context *restrict c,
                            Instruction I,
                            Type const *result_type,
                            Type const *lhs_type,
                            Type const *rhs_type) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = context_lookup_ssa(c, I.A_data.ssa);
    Type const    *Bty;
    if (!typecheck_operand(&Bty, c, I.B_kind, I.B_data)) { return false; }
    if (!type_equality(lhs_type, Bty)) {
        return failure_mismatch_type(c, lhs_type, Bty);
    }
    Type const *Cty;
    if (!typecheck_operand(&Cty, c, I.C_kind, I.C_data)) { return false; }
    if (!type_equality(rhs_type, Cty)) {
        return failure_mismatch_type(c, rhs_type, Cty);
    }
    local->type = result_type;
    return success(result, result_type);
}

static bool
typecheck_add(Type const **result, Context *restrict c, Instruction I) {
    if (context_trace(c)) { trace(SV("typecheck_add:"), stdout); }
    Type const *type_i64 = context_i64_type(c);
    return typecheck_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool
typecheck_sub(Type const **result, Context *restrict c, Instruction I) {
    if (context_trace(c)) { trace(SV("typecheck_sub:"), stdout); }
    Type const *type_i64 = context_i64_type(c);
    return typecheck_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool
typecheck_mul(Type const **result, Context *restrict c, Instruction I) {
    if (context_trace(c)) { trace(SV("typecheck_mul:"), stdout); }
    Type const *type_i64 = context_i64_type(c);
    return typecheck_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool
typecheck_div(Type const **result, Context *restrict c, Instruction I) {
    if (context_trace(c)) { trace(SV("typecheck_div:"), stdout); }
    Type const *type_i64 = context_i64_type(c);
    return typecheck_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool
typecheck_mod(Type const **result, Context *restrict c, Instruction I) {
    if (context_trace(c)) { trace(SV("typecheck_mod:"), stdout); }
    Type const *type_i64 = context_i64_type(c);
    return typecheck_binop(result, c, I, type_i64, type_i64, type_i64);
}

static bool typecheck_function(Type const **result, Context *restrict c) {
    Type const *return_type = NULL;
    Function   *body        = context_current_function(c);
    Bytecode   *bc          = &body->bc;

    Instruction *ip = bc->buffer;
    for (u16 idx = 0; idx < bc->length; ++idx) {
        Instruction I = ip[idx];
        switch (I.opcode) {
        case OPCODE_RET: {
            Type const *Bty;
            if (!typecheck_ret(&Bty, c, I)) { return false; }

            if ((return_type != NULL) && (!type_equality(return_type, Bty))) {
                return failure_mismatch_type(c, return_type, Bty);
            }

            return_type = Bty;
            break;
        }

        case OPCODE_CALL: {
            Type const *Aty;
            if (!typecheck_call(&Aty, c, I)) { return false; }
            break;
        }

        case OPCODE_LOAD: {
            Type const *Aty;
            if (!typecheck_load(&Aty, c, I)) { return false; }
            break;
        }

        case OPCODE_DOT: {
            Type const *Aty;
            if (!typecheck_dot(&Aty, c, I)) { return false; }
            break;
        }

        case OPCODE_NEG: {
            Type const *Aty;
            if (!typecheck_neg(&Aty, c, I)) { return false; }
            break;
        }

        case OPCODE_ADD: {
            Type const *Aty;
            if (!typecheck_add(&Aty, c, I)) { return false; }
            break;
        }

        case OPCODE_SUB: {
            Type const *Aty;
            if (!typecheck_sub(&Aty, c, I)) { return false; }
            break;
        }

        case OPCODE_MUL: {
            Type const *Aty;
            if (!typecheck_mul(&Aty, c, I)) { return false; }
            break;
        }

        case OPCODE_DIV: {
            Type const *Aty;
            if (!typecheck_div(&Aty, c, I)) { return false; }
            break;
        }

        case OPCODE_MOD: {
            Type const *Aty;
            if (!typecheck_mod(&Aty, c, I)) { return false; }
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }

    return success(result, return_type);
}

static bool typecheck_global(Type const **result,
                             Context *restrict c,
                             Symbol *restrict element) {
    if (context_trace(c)) { trace(SV("typecheck_global:"), stdout); }
    assert(c != nullptr);
    assert(element != nullptr);
    if (element->type != NULL) { return success(result, element->type); }

    switch (element->kind) {
    case STE_UNDEFINED: {
        if (context_trace(c)) {
            trace(SV("typecheck_global: undefined"), stdout);
        }
        // #TODO: this should be handled as a forward declaration
        // but only if the type exists.
        return success(result, context_nil_type(c));
    }

    case STE_FUNCTION: {
        if (context_trace(c)) {
            trace(SV("typecheck_global: function:"), stdout);
            trace(element->name, stdout);
        }
        // we want to avoid infinite recursion. but we also need to
        // handle the fact that functions are going to be typechecked
        // in an indeterminite order. the natural solution is to type
        // the dependencies of a function body as those are used within
        // the function body. This only breaks when we have mutual recursion,
        // otherwise, when the global is successfully typed.
        // the question is, how do we accomplish this?
        Function   *body = context_enter_function(c, element->name);
        Type const *Rty;
        if (!typecheck_function(&Rty, c)) {
            context_leave_function(c);
            return false;
        }
        context_leave_function(c);

        if ((body->return_type != NULL) &&
            (!type_equality(Rty, body->return_type))) {
            return failure_mismatch_type(c, body->return_type, Rty);
        }

        body->return_type         = Rty;
        Type const *function_type = type_of_function(body, c);
        element->type             = function_type;
        return success(result, function_type);
    }

    default: EXP_UNREACHABLE();
    }
}

#undef try

i32 typecheck(Context *restrict context) {
    if (context_trace(context)) {
        trace(SV("typecheck:"), stdout);
        trace(context_source_path(context), stdout);
    }
    i32                 result = EXIT_SUCCESS;
    SymbolTableIterator iter   = context_global_symbol_table_iterator(context);
    while (!symbol_table_iterator_done(&iter)) {
        Type const *type = NULL;
        if (!typecheck_global(&type, context, (*iter.element))) {
            Error *error = context_current_error(context);
            error_print(error, context_source_path(context), 0);
            error_destroy(error);
            result |= EXIT_FAILURE;
        }

        symbol_table_iterator_next(&iter);
    }

    return result;
}

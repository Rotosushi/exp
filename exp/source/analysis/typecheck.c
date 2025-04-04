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
    LocalVariable *local = context_lookup_ssa(context, I.A_data.ssa);
    if (!typecheck_operand(&local->type, context, I.B_kind, I.B_data)) {
        return false;
    }
    return success(result, local->type);
}

static bool
typecheck_ret(Type const **result, Context *restrict context, Instruction I) {
    return typecheck_operand(result, context, I.B_kind, I.B_data);
}

static bool
typecheck_call(Type const **result, Context *restrict context, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
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

static bool tuple_index_out_of_bounds(u64 index, TupleType *tuple) {
    return index >= tuple->size;
}

static bool
typecheck_dot(Type const **result, Context *restrict context, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = context_lookup_ssa(context, I.A_data.ssa);
    Type const    *Bty;
    if (!typecheck_operand(&Bty, context, I.B_kind, I.B_data)) { return false; }

    if (Bty->kind != TYPE_KIND_TUPLE) {
        String buf = string_create();
        string_append(&buf, SV("Type is not a tuple ["));
        print_type(&buf, Bty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    TupleType *tuple = &Bty->tuple_type;
    Operand    C     = operand(I.C_kind, I.C_data);

    if (!operand_is_index(C)) {
        return error(ERROR_TYPECHECK_TUPLE_INDEX_NOT_IMMEDIATE,
                     string_from_view(SV("")));
    }

    u64 index = operand_as_index(C);

    if (tuple_index_out_of_bounds(index, tuple)) {
        String buf = string_create();
        string_append(&buf, SV("The given index "));
        string_append_u64(&buf, index);
        string_append(&buf, SV(" is not in the valid range of 0.."));
        string_append_u64(&buf, tuple->size);
        return error(ERROR_TYPECHECK_TUPLE_INDEX_OUT_OF_BOUNDS, buf);
    }

    local->type = tuple->types[index];
    return success(tuple->types[index]);
}

static TResult typecheck_neg(Context *restrict c, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = context_lookup_ssa(c, I.A_data.ssa);
    try(Bty, typecheck_operand(c, I.B_kind, I.B_data));

    Type *i64ty = context_i64_type(c);
    if (!type_equality(i64ty, Bty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Bty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    local->type = Bty;
    return success(Bty);
}

static TResult typecheck_add(Context *restrict c, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = context_lookup_ssa(c, I.A_data.ssa);
    try(Bty, typecheck_operand(c, I.B_kind, I.B_data));

    try(Cty, typecheck_operand(c, I.C_kind, I.C_data));

    Type *i64ty = context_i64_type(c);
    if (!type_equality(i64ty, Bty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Bty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    if (!type_equality(Bty, Cty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Cty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    local->type = Bty;
    return success(Bty);
}

static TResult typecheck_sub(Context *restrict c, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = context_lookup_ssa(c, I.A_data.ssa);
    try(Bty, typecheck_operand(c, I.B_kind, I.B_data));

    try(Cty, typecheck_operand(c, I.C_kind, I.C_data));

    Type *i64ty = context_i64_type(c);
    if (!type_equality(i64ty, Bty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Bty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    if (!type_equality(Bty, Cty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Cty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    local->type = Bty;
    return success(Bty);
}

static TResult typecheck_mul(Context *restrict c, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = context_lookup_ssa(c, I.A_data.ssa);
    try(Bty, typecheck_operand(c, I.B_kind, I.B_data));

    try(Cty, typecheck_operand(c, I.C_kind, I.C_data));

    Type *i64ty = context_i64_type(c);
    if (!type_equality(i64ty, Bty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Bty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    if (!type_equality(Bty, Cty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Cty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    local->type = Bty;
    return success(Bty);
}

static TResult typecheck_div(Context *restrict c, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = context_lookup_ssa(c, I.A_data.ssa);
    try(Bty, typecheck_operand(c, I.B_kind, I.B_data));

    try(Cty, typecheck_operand(c, I.C_kind, I.C_data));

    Type *i64ty = context_i64_type(c);
    if (!type_equality(i64ty, Bty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Bty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    if (!type_equality(Bty, Cty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Cty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    local->type = Bty;
    return success(Bty);
}

static TResult typecheck_mod(Context *restrict c, Instruction I) {
    assert(I.A_kind == OPERAND_KIND_SSA);
    LocalVariable *local = context_lookup_ssa(c, I.A_data.ssa);
    try(Bty, typecheck_operand(c, I.B_kind, I.B_data));

    try(Cty, typecheck_operand(c, I.C_kind, I.C_data));

    Type *i64ty = context_i64_type(c);
    if (!type_equality(i64ty, Bty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Bty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    if (!type_equality(Bty, Cty)) {
        String buf = string_create();
        string_append(&buf, SV("Expected [i64] Actual ["));
        print_type(&buf, Cty);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }

    local->type = Bty;
    return success(Bty);
}

static TResult typecheck_function(Context *restrict c) {
    Type     *return_type = NULL;
    Function *body        = context_current_function(c);
    Bytecode *bc          = &body->bc;

    Instruction *ip = bc->buffer;
    for (u16 idx = 0; idx < bc->length; ++idx) {
        Instruction I = ip[idx];
        switch (I.opcode) {
        case OPCODE_RET: {
            try(Bty, typecheck_ret(c, I));

            if ((return_type != NULL) && (!type_equality(return_type, Bty))) {
                String buf = string_create();
                string_append(&buf, SV("Previous return statement had type ["));
                print_type(&buf, return_type);
                string_append(&buf, SV("] this return statement has type ["));
                print_type(&buf, Bty);
                string_append(&buf, SV("]"));
                return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
            }

            return_type = Bty;
            break;
        }

        case OPCODE_CALL: {
            try(Aty, typecheck_call(c, I));
            (void)Aty;
            break;
        }

        case OPCODE_DOT: {
            try(Aty, typecheck_dot(c, I));
            (void)Aty;
            break;
        }

        case OPCODE_LOAD: {
            try(Bty, typecheck_load(c, I));
            (void)Bty;
            break;
        }

        case OPCODE_NEG: {
            try(Bty, typecheck_neg(c, I));
            (void)Bty;
            break;
        }

        case OPCODE_ADD: {
            try(Aty, typecheck_add(c, I));
            (void)Aty;
            break;
        }

        case OPCODE_SUB: {
            try(Aty, typecheck_sub(c, I));
            (void)Aty;
            break;
        }

        case OPCODE_MUL: {
            try(Aty, typecheck_mul(c, I));
            (void)Aty;
            break;
        }

        case OPCODE_DIV: {
            try(Aty, typecheck_div(c, I));
            (void)Aty;
            break;
        }

        case OPCODE_MOD: {
            try(Aty, typecheck_mod(c, I));
            (void)Aty;
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }

    return success(return_type);
}

static TResult typecheck_global(Context *restrict c, Symbol *restrict element) {
    assert(c != nullptr);
    assert(element != nullptr);
    if (element->type != NULL) { return success(element->type); }

    switch (element->kind) {
    case STE_UNDEFINED: {
        // #TODO: this should be handled as a forward declaration
        // but only if the type exists.
        return success(context_nil_type(c));
    }

    case STE_FUNCTION: {
        // we want to avoid infinite recursion. but we also need to
        // handle the fact that functions are going to be typechecked
        // in an indeterminite order. the natural solution is to type
        // the dependencies of a function body as those are used within
        // the function body. This only breaks when we have mutual recursion,
        // otherwise, when the global is successfully typed.
        // the question is, how do we accomplish this?
        Function *body = context_enter_function(c, element->name);

        try(Rty, typecheck_function(c));
        context_leave_function(c);

        if ((body->return_type != NULL) &&
            (!type_equality(Rty, body->return_type))) {
            String buf = string_create();
            string_append(&buf, SV("Function was annotated with type ["));
            print_type(&buf, body->return_type);
            string_append(&buf, SV("] actual returned type ["));
            print_type(&buf, Rty);
            string_append(&buf, SV("]"));
            return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
        }

        body->return_type   = Rty;
        Type *function_type = type_of_function(body, c);
        element->type       = function_type;
        return success(function_type);
    }

    default: EXP_UNREACHABLE();
    }
}

#undef try

i32 typecheck(Context *restrict context) {
    i32                 result = EXIT_SUCCESS;
    SymbolTableIterator iter   = context_global_symbol_table_iterator(context);
    while (!symbol_table_iterator_done(&iter)) {
        TResult tr = typecheck_global(context, (*iter.element));
        if (tr.has_error) {
            error_print(&tr.error, context_source_path(context), 0);
            tresult_destroy(&tr);
            result |= EXIT_FAILURE;
        }

        symbol_table_iterator_next(&iter);
    }

    return result;
}

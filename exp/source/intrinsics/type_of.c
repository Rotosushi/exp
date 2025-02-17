/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include <assert.h>

#include "imr/operand.h"
#include "imr/scalar.h"
#include "intrinsics/type_of.h"
#include "utility/panic.h"
#include "utility/unreachable.h"

Type const *type_of_scalar(Scalar scalar, Context *context) {
    assert(context != nullptr);
    switch (scalar.kind) {
    case SCALAR_NIL:  return context_nil_type(context);
    case SCALAR_BOOL: return context_bool_type(context);
    case SCALAR_I8:   return context_i8_type(context);
    case SCALAR_I16:  return context_i16_type(context);
    case SCALAR_I32:  return context_i32_type(context);
    case SCALAR_I64:  return context_i64_type(context);
    case SCALAR_U8:   return context_u8_type(context);
    case SCALAR_U16:  return context_u16_type(context);
    case SCALAR_U32:  return context_u32_type(context);
    case SCALAR_U64:  return context_u64_type(context);
    default:          EXP_UNREACHABLE();
    }
}

Type const *type_of_operand(Operand operand, Function *function,
                            Context *context) {
    assert(function != nullptr);
    assert(context != nullptr);
    switch (operand.kind) {
    case OPERAND_SSA: {
        Local *local = function_local_at(function, operand.data.ssa);
        assert(local != nullptr);
        assert(local->type != nullptr);
        return local->type;
    }

    case OPERAND_SCALAR: {
        return type_of_scalar(operand.data.scalar, context);
    }

    case OPERAND_CONSTANT: {
        Value *constant = context_constants_at(context, operand.data.constant);
        return type_of_value(constant, function, context);
    }

    case OPERAND_LABEL: {
        StringView name = context_labels_at(context, operand.data.label);
        Symbol *global  = context_symbol_table_at(context, name);
        assert(global->type != nullptr);
        return global->type;
    }

    default: EXP_UNREACHABLE();
    }
}

Type const *type_of_value(Value *value, Function *function, Context *context) {
    assert(value != nullptr);
    assert(function != nullptr);
    assert(context != nullptr);
    switch (value->kind) {
    case VALUE_UNINITIALIZED: PANIC("uninitialized Value");
    case VALUE_SCALAR:        return type_of_scalar(value->scalar, context);
    case VALUE_TUPLE:         {
        Tuple *tuple = &value->tuple;
        TupleType tuple_type;
        tuple_type_initialize(&tuple_type);
        for (u64 i = 0; i < tuple->size; ++i) {
            Operand element = tuple->elements[i];
            Type const *T   = type_of_operand(element, function, context);
            tuple_type_append(&tuple_type, T);
        }
        return context_tuple_type(context, tuple_type);
    }

    default: EXP_UNREACHABLE();
    }
}

Type const *type_of_function(Function *body, Context *context) {
    assert(body != nullptr);
    assert(body->return_type != nullptr);

    TupleType argument_types;
    tuple_type_initialize(&argument_types);
    for (u64 index = 0; index < body->arguments.length; ++index) {
        Local *formal_argument    = body->arguments.buffer + index;
        Type const *argument_type = formal_argument->type;
        tuple_type_append(&argument_types, argument_type);
    }

    return context_function_type(context, body->return_type, argument_types);
}

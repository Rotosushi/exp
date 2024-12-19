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

#include "intrinsics/type_of.h"
#include "utility/panic.h"
#include "utility/unreachable.h"

Type const *type_of_operand(OperandKind kind,
                            OperandData data,
                            FunctionBody *function,
                            Context *context) {
    assert(function != nullptr);
    assert(context != nullptr);
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_body_local_at(function, data.ssa);
        assert(local->type != nullptr);
        return local->type;
    }

    case OPERAND_KIND_I32: {
        return context_i32_type(context);
    }
    case OPERAND_KIND_CONSTANT: {
        Value *value = context_constants_at(context, data.constant);
        return type_of_value(value, function, context);
    }

    case OPERAND_KIND_LABEL: {
        StringView name = context_labels_at(context, data.label);
        Local *local    = function_body_local_at_name(function, name);
        if (local != nullptr) {
            assert(local->type != nullptr);
            return local->type;
        }

        Symbol *global = context_symbol_table_at(context, name);
        assert(global->type != nullptr);
        return global->type;
    }

    default: EXP_UNREACHABLE();
    }
}

Type const *
type_of_value(Value *value, FunctionBody *function, Context *context) {
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: PANIC("uninitialized Value");
    case VALUE_KIND_NIL:           return context_nil_type(context);
    case VALUE_KIND_BOOLEAN:       return context_boolean_type(context);
    case VALUE_KIND_I32:           return context_i32_type(context);
    case VALUE_KIND_TUPLE:         {
        Tuple *tuple = &value->tuple;
        TupleType tuple_type;
        tuple_type_initialize(&tuple_type);
        for (u64 i = 0; i < tuple->size; ++i) {
            Operand element = tuple->elements[i];
            Type const *T =
                type_of_operand(element.kind, element.data, function, context);
            tuple_type_append(&tuple_type, T);
        }
        return context_tuple_type(context, tuple_type);
    }

    default: EXP_UNREACHABLE();
    }
}

Type const *type_of_function(FunctionBody *body, Context *context) {
    assert(body != NULL);
    assert(body->return_type != NULL);

    TupleType argument_types;
    tuple_type_initialize(&argument_types);
    for (u64 i = 0; i < body->arguments.size; ++i) {
        FormalArgument *formal_argument = &body->arguments.list[i];
        Type const *argument_type       = formal_argument->type;
        tuple_type_append(&argument_types, argument_type);
    }

    return context_function_type(context, body->return_type, argument_types);
}

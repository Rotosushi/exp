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

#include "env/context.h"
#include "intrinsics/type_of.h"
#include "support/assert.h"
#include "support/panic.h"
#include "support/unreachable.h"

Type const *type_of_value(Value const *restrict value,
                          Function const *restrict function,
                          Context *restrict context) {
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: PANIC("uninitialized Value");
    case VALUE_KIND_NIL:           return context_nil_type(context);
    case VALUE_KIND_BOOLEAN:       return context_bool_type(context);
    case VALUE_KIND_U8:            return context_u8_type(context);
    case VALUE_KIND_U16:           return context_u16_type(context);
    case VALUE_KIND_U32:           return context_u32_type(context);
    case VALUE_KIND_U64:           return context_u64_type(context);
    case VALUE_KIND_I8:            return context_i8_type(context);
    case VALUE_KIND_I16:           return context_i16_type(context);
    case VALUE_KIND_I32:           return context_i32_type(context);
    case VALUE_KIND_I64:           return context_i64_type(context);

    case VALUE_KIND_TUPLE:
        return type_of_tuple(&value->tuple, function, context);

    case VALUE_KIND_FUNCTION:
        return type_of_function(&value->function, context);

    default: EXP_UNREACHABLE();
    }
}

Type const *type_of_function(Function const *restrict body,
                             Context *restrict context) {
    assert(body != NULL);
    assert(body->return_type != NULL);

    TupleType argument_types = tuple_type_create();
    for (u64 i = 0; i < body->arguments.size; ++i) {
        Local      *formal_argument = body->arguments.list[i];
        Type const *argument_type   = formal_argument->type;
        tuple_type_append(&argument_types, argument_type);
    }

    return context_function_type(context, body->return_type, argument_types);
}

Type const *type_of_tuple(Tuple const *restrict tuple,
                          Function const *restrict function,
                          Context *restrict context) {
    TupleType tuple_type = tuple_type_create();
    for (u64 i = 0; i < tuple->size; ++i) {
        Type const *T = type_of_operand(tuple->elements[i], function, context);
        tuple_type_append(&tuple_type, T);
    }
    return context_tuple_type(context, tuple_type);
}

Type const *type_of_operand(Operand operand,
                            Function const *restrict function,
                            Context *restrict context) {
    switch (operand.kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_lookup_local(function, operand.data.ssa);
        exp_assert_debug(local->type != NULL);
        return local->type;
    }

    case OPERAND_KIND_CONSTANT:
        return type_of_value(operand.data.constant, function, context);

    case OPERAND_KIND_LABEL: {
        StringView label = constant_string_to_view(operand.data.label);

        Local *local = function_lookup_local_name(function, label);
        if (local != NULL) {
            exp_assert_debug(local->type != NULL);
            return local->type;
        }

        Symbol *symbol = context_global_symbol_lookup(context, label);
        assert(!string_view_empty(symbol->name));
        assert(symbol->type != NULL);
        return symbol->type;
    }

    case OPERAND_KIND_NIL:  return context_nil_type(context);
    case OPERAND_KIND_BOOL: return context_bool_type(context);
    case OPERAND_KIND_U8:   return context_u8_type(context);
    case OPERAND_KIND_U16:  return context_u16_type(context);
    case OPERAND_KIND_U32:  return context_u32_type(context);
    case OPERAND_KIND_U64:  return context_u64_type(context);
    case OPERAND_KIND_I8:   return context_i8_type(context);
    case OPERAND_KIND_I16:  return context_i16_type(context);
    case OPERAND_KIND_I32:  return context_i32_type(context);
    case OPERAND_KIND_I64:  return context_i64_type(context);

    default: EXP_UNREACHABLE();
    }
}

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
#include "support/panic.h"
#include "support/unreachable.h"

Type const *type_of_value(Value *restrict value, Context *restrict context) {
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: PANIC("uninitialized Value");
    case VALUE_KIND_NIL:           return context_nil_type(context);
    case VALUE_KIND_BOOLEAN:       return context_boolean_type(context);
    case VALUE_KIND_I64:           return context_i64_type(context);
    case VALUE_KIND_TUPLE:         {
        Tuple    *tuple      = &value->tuple;
        TupleType tuple_type = tuple_type_create();
        for (u64 i = 0; i < tuple->size; ++i) {
            Type const *T = type_of_operand(tuple->elements[i], context);
            tuple_type_append(&tuple_type, T);
        }
        return context_tuple_type(context, tuple_type);
    }

    default: EXP_UNREACHABLE();
    }
}

Type const *type_of_function(Function *restrict body,
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

Type const *type_of_operand(Operand operand, Context *restrict context) {
    switch (operand.kind) {
    case OPERAND_KIND_SSA: {
        Local *local = context_lookup_local(context, operand.data.ssa);
        return local->type;
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *constant = context_constants_at(context, operand.data.constant);
        return type_of_value(constant, context);
        break;
    }

    case OPERAND_KIND_I64: {
        // #TODO: we can theoretically fit a u8 and a i8 in an
        //  immediate as well. so maybe let's do that?
        return context_i64_type(context);
        break;
    }

    case OPERAND_KIND_LABEL: {
        StringView label  = constant_string_to_view(operand.data.label);
        Symbol    *symbol = context_global_symbol_table_at(context, label);
        assert(!string_view_empty(symbol->name));
        assert(symbol->type != NULL);
        return symbol->type;
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

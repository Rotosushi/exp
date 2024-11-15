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
#include "utility/unreachable.h"

Type *type_of_value(Value *restrict value, Context *restrict context) {
  switch (value->kind) {
  case VALUEKIND_UNINITIALIZED: PANIC("uninitialized Value");
  case VALUEKIND_NIL:           return context_nil_type(context);
  case VALUEKIND_BOOLEAN:       return context_boolean_type(context);
  case VALUEKIND_I64:           return context_i64_type(context);
  case VALUEKIND_TUPLE:         {
    Tuple *tuple         = &value->tuple;
    TupleType tuple_type = tuple_type_create();
    for (u64 i = 0; i < tuple->size; ++i) {
      Type *T = type_of_operand(tuple->elements + i, context);
      tuple_type_append(&tuple_type, T);
    }
    return context_tuple_type(context, tuple_type);
  }

  default: EXP_UNREACHABLE();
  }
}

Type *type_of_function(FunctionBody *restrict body, Context *restrict context) {
  assert(body != NULL);
  assert(body->return_type != NULL);

  TupleType argument_types = tuple_type_create();
  for (u64 i = 0; i < body->arguments.size; ++i) {
    FormalArgument *formal_argument = &body->arguments.list[i];
    Type *argument_type             = formal_argument->type;
    tuple_type_append(&argument_types, argument_type);
  }

  return context_function_type(context, body->return_type, argument_types);
}

Type *type_of_operand(Operand *restrict operand, Context *restrict context) {
  switch (operand->format) {
  case OPERAND_KIND_SSA: {
    LocalVariable *local = context_lookup_ssa(context, operand->ssa);
    return local->type;
    break;
  }

  case OPERAND_KIND_VALUE: {
    Value *constant = context_values_at(context, operand->index);
    return type_of_value(constant, context);
    break;
  }

  case OPERAND_KIND_IMMEDIATE: {
    return context_i64_type(context);
    break;
  }

  case OPERAND_KIND_LABEL: {
    StringView label = context_global_labels_at(context, operand->index);
    SymbolTableElement *symbol = context_global_symbol_table_at(context, label);
    assert(!string_view_empty(symbol->name));
    assert(symbol->type != NULL);
    return symbol->type;
    break;
  }

  default: EXP_UNREACHABLE();
  }
}

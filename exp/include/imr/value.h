// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_IMR_VALUE_H
#define EXP_IMR_VALUE_H

#include "imr/function.h"
#include "imr/type.h"
#include "utility/string_view.h"

typedef enum ValueKind {
  VALUEKIND_UNINITIALIZED,

  VALUEKIND_NIL,
  VALUEKIND_BOOLEAN,
  VALUEKIND_I64,
} ValueKind;

/**
 * @brief represents Values in the compiler
 *
 */
typedef struct Value {
  ValueKind kind;
  union {
    bool nil;
    bool boolean;
    i64 integer;
  };
} Value;

/**
 * @brief create an uninitialized value
 *
 * @return Value
 */
Value value_create();

/**
 * @brief create a nil value
 *
 * @return Value
 */
Value value_create_nil();

/**
 * @brief create a boolean value
 *
 * @param b
 * @return Value
 */
Value value_create_boolean(bool b);

/**
 * @brief create an Integer value
 *
 * @param i
 * @return Value
 */
Value value_create_integer(i64 i);

/**
 * @brief assign dest the value of source
 *
 * @param dest
 * @param source
 */
void value_assign(Value *dest, Value *source);

/**
 * @brief equality compares values
 *
 * @param v1
 * @param v2
 * @return true
 * @return false
 */
bool value_equality(Value *v1, Value *v2);

#endif // !EXP_IMR_VALUE_H
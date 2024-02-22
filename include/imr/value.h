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

typedef enum ValueKind {
  VALUEKIND_INTEGER,
  // VALUEKIND_BOOLEAN,
  // VALUEKIND_NIL,
} ValueKind;

typedef struct Value {
  ValueKind kind;
  union {
    long integer;
    // bool boolean;
    // bool nil;
  };
} Value;

Value value_create_integer(long i);

#endif // !EXP_IMR_VALUE_H
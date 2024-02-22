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
#ifndef EXP_IMR_TYPE_H
#define EXP_IMR_TYPE_H

typedef enum TypeKind {
  TYPEKIND_INTEGER,
  // TYPEKIND_BOOLEAN,
  // TYPEKIND_NIL,
  // TYPEKIND_FUNCTION,
} TypeKind;

typedef struct IntegerType {
  char empty; // zero length structs are not valid C
} IntegerType;

// typedef struct BooleanType {
//   char empty;
// } BooleanType;

// typedef struct NilType {
//   char empty;
// } NilType;

// struct Type;
// typedef struct FunctionType {
//   Type *return_type;
//   size_t argument_count;
//   Type *argument_types;
// };

typedef struct Type {
  TypeKind kind;
  union {
    IntegerType integer_type;
    // BooleanType boolean_type;
    // NilType nil_type;
    // FunctionType function_type;
  };
} Type;

Type type_create_integer();

#endif // !EXP_IMR_TYPE_H
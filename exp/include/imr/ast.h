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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_IMR_AST_H
#define EXP_IMR_AST_H

#include "imr/value.h"
#include "utility/string_view.h"

struct Ast;

typedef struct Constant {
  StringView name;
  struct Ast *right;
} Constant;

typedef enum Operator {
  OPR_ADD,
  OPR_SUB,
  OPR_MUL,
  OPR_DIV,
  OPR_MOD,
} Operator;

typedef struct Unop {
  Operator op;
  struct Ast *right;
} Unop;

typedef struct Binop {
  Operator op;
  struct Ast *left;
  struct Ast *right;
} Binop;

typedef enum AstKind {
  ASTKIND_VALUE,
  ASTKIND_UNOP,
  ASTKIND_BINOP,
  ASTKIND_CONSTANT,
} AstKind;

typedef struct Ast {
  AstKind kind;
  union {
    Value value;
    Unop unop;
    Binop binop;
    Constant constant;
  };
} Ast;

void ast_destroy(Ast *restrict ast);

Ast *ast_create_value(Value value);
Ast *ast_create_unop(Operator op, Ast *restrict right);
Ast *ast_create_binop(Operator op, Ast *restrict left, Ast *restrict right);
Ast *ast_create_constant(StringView name, Ast *restrict right);

#endif // !EXP_IMR_AST_H
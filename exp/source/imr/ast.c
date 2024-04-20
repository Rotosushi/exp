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
#include <stdlib.h>

#include "imr/ast.h"
#include "utility/alloc.h"
#include "utility/panic.h"

void ast_destroy(Ast *restrict ast) {
  switch (ast->kind) {
  case ASTKIND_VALUE:
    break;

  case ASTKIND_UNOP:
    ast_destroy(ast->unop.right);
    break;

  case ASTKIND_BINOP:
    ast_destroy(ast->binop.left);
    ast_destroy(ast->binop.right);
    break;

  case ASTKIND_CONSTANT:
    ast_destroy(ast->constant.right);
    break;

  default:
    PANIC("bad ASTKIND");
  }
  free(ast);
}

Ast *ast_create_value(Value value) {
  Ast *ast   = allocate(sizeof(Ast));
  ast->kind  = ASTKIND_VALUE;
  ast->value = value;
  return ast;
}

Ast *ast_create_unop(Operator op, Ast *restrict right) {
  Ast *ast        = allocate(sizeof(Ast));
  ast->kind       = ASTKIND_UNOP;
  ast->unop.op    = op;
  ast->unop.right = right;
  return ast;
}

Ast *ast_create_binop(Operator op, Ast *restrict left, Ast *restrict right) {
  Ast *ast         = allocate(sizeof(Ast));
  ast->kind        = ASTKIND_BINOP;
  ast->binop.op    = op;
  ast->binop.left  = left;
  ast->binop.right = right;
  return ast;
}

Ast *ast_create_constant(StringView name, Ast *restrict right) {
  Ast *ast            = allocate(sizeof(Ast));
  ast->kind           = ASTKIND_CONSTANT;
  ast->constant.name  = name;
  ast->constant.right = right;
  return ast;
}

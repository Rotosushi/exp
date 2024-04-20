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
#ifndef EXP_ENV_MODULE_H
#define EXP_ENV_MODULE_H

#include "imr/ast.h"

typedef struct Module {
  u64 size;
  u64 capacity;
  Ast **list;
} Module;

Module module_create();
void module_destroy(Module *restrict m);

/**
 * @brief add an ast to the module
 *
 * @note takes ownership of the ast
 *
 * @param m
 * @param a
 */
void module_add(Module *restrict m, Ast *restrict a);

#endif // !EXP_ENV_MODULE_H
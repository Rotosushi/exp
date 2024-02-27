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
#ifndef EXP_ENV_CONTEXT_H
#define EXP_ENV_CONTEXT_H

#include "env/string_interner.h"
#include "env/type_interner.h"
#include "imr/bytecode.h"
#include "imr/constants.h"
#include "imr/registers.h"

typedef struct Context {
  StringInterner string_interner;
  TypeInterner type_interner;
  Constants constants;
  Bytecode global_bytecode;
  Registers registers;
} Context;

#endif // !EXP_ENV_CONTEXT_H
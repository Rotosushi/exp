// Copyright (C) 2025 Cade Weinberg
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

#ifndef EXP_CODEGEN_X86_FUNCTION_H
#define EXP_CODEGEN_X86_FUNCTION_H

#include "codegen/x86/imr/function.h"
#include "env/context.h"
#include "imr/value.h"

void print_x86_function(String *restrict buffer,
                        Value const *restrict value,
                        Context *restrict context);

#endif // !EXP_CODEGEN_X86_FUNCTION_H

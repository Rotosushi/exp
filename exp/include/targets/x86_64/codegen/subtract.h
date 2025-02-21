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

/**
 * @file targets/x86_64/codegen/subtract.h
 */

#ifndef EXP_TARGETS_X86_64_CODEGEN_SUBTRACT_H
#define EXP_TARGETS_X86_64_CODEGEN_SUBTRACT_H

#include "env/context.h"
#include "utility/result.h"

ExpResult x86_64_codegen_subtract(String *buffer, Instruction *instruction,
                                  Function *function, Context *context);

#endif // EXP_TARGETS_X86_64_CODEGEN_SUBTRACT_H

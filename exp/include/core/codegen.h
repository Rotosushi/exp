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
 * @file core/codegen.h
 */

#ifndef EXP_CODEGEN_CODEGEN_H
#define EXP_CODEGEN_CODEGEN_H
#include "env/context.h"

/**
 * @brief Converts the IR in the given context into x86_64 assembly.
 * writes the assembly to the file specified by context_assembly_path(context).
 */
void codegen(Context *restrict context);

#endif // !EXP_CODEGEN_CODEGEN_H

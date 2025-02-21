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
 * @file core/assemble.h
 */

#ifndef EXP_CORE_ASSEMBLE_H
#define EXP_CORE_ASSEMBLE_H
#include "env/context.h"
#include "utility/result.h"

/**
 * @brief Assembles the given context into an executable.
 *
 * calls the system assembler "as" to assemble the file
 * given by context_assemly_path(context) into an object file
 * located at context_object_path(context).
 *
 * @note we could also embed an assembler into the compiler, something like
 * zydis. I am trying to reduce the number of dependencies in the
 * compiler, and calling "as" allows a user to supply a different
 * assembler. Though we only emit AT&T syntax.
 *
 * @warning expects the context to contain a main subroutine.
 * currently the linker produces an error if there isn't a
 * main subroutine.
 */
ExpResult assemble(Context *context);

#endif // !EXP_CORE_ASSEMBLE_H

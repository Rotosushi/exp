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
#ifndef EXP_BACKEND_EMIT_X64_LINUX_ASSEMBLY_H
#define EXP_BACKEND_EMIT_X64_LINUX_ASSEMBLY_H

#include "env/context.h"
#include "utility/string_view.h"

/*
  #NOTE: as far as I can tell, defining symbols is nearly
  the entire job of writing assembly for the assembler
  and linker.

  globals are symbols
  functions are globals
  constants are globals
  variables are globals

  then there is information like debug info
  and what compiler produced the output and
  the source file, etc...)

  program execution in its most primitive form is:

  os-scheduler -> main() -> ... -> main() -> os-scheduler

  and anything that main can do is done by way of
  expressions occuring within the body of main.

  expressions are transformed into assembly instructions,
  either encoding literal values as immediates, or using
  globally defined constants/variables, or calling globally
  defined functions.
*/

/**
 * @brief writes assembly to <path> defining each symbol
 * within <context>'s global_symbols
 *
 * @param symbol_table
 * @param path
 */
// void emit_x64_linux_assembly(Context *restrict context);

#endif // !EXP_BACKEND_EMIT_X64_LINUX_ASSEMBLY_H
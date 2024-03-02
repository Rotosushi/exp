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
#ifndef EXP_BACKEND_EMITTER_H
#define EXP_BACKEND_EMITTER_H

#include "env/context.h"
#include "utility/string_view.h"

/*
  #NOTE: as far as I can tell, defining symbols is
  the entire job of writing assembly for the assembler
  and linker.

  globals are symbols
  functions are symbols

  program execution in its most primitive form is:

  os -> main() -> ... -> main() -> os

  and anything that main can do is done by way of symbols,
  expressions, and constants.

  expressions are transformed into assembly, that is only
  accessable by way of calling a function. and constants
  are transformed into global initializers, and into
  components of those same assembly instructions.

  thus, defining global constants is like 1/4 or 1/3 or 1/2
  (depending on how you slice the compiler)
  of the entire compiler. (modulo types)

  Bytecode, Registers, and the VirtualMachine, are all components
  of an interpreter, not a compiler.
  their existence paves a way for compile time execution.
  but they are not the code that emits assembly instructions.
*/

/**
 * @brief writes assembly to <path> defining each symbol
 * within <context>'s global_symbols
 *
 * @param symbol_table
 * @param path
 */
void emit(Context *restrict context);

#endif // !EXP_BACKEND_EMITTER_H
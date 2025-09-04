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
#ifndef EXP_ENV_RUNTIME_H
#define EXP_ENV_RUNTIME_H

#include "env/symbol_table.h"

/**
 * @brief add the symbols available to the runtime of an exp program
 * to the given SymbolTable.
 *
 * This is done such that the codegenerator can construct function calls
 * to library routines with the same machinery it uses to construct
 * function calls to user code.
 */
void initialize_runtime_symbols(SymbolTable *restrict table);

#endif // !EXP_ENV_RUNTIME_H

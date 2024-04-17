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
#ifndef EXP_CORE_INTERPRET_H
#define EXP_CORE_INTERPRET_H
#include "env/context.h"
#include "env/error.h"

/**
 * @brief interpret the given context.
 *
 * @note this is -the- intermediate step, which does all
 * typechecking and compile time evaluation.
 * crucially it creates the symbol table entries
 * which the backend uses to direct emission
 * of global symbols into the output assembly file.
 * it may be that "interpret" isn't the best name
 * given that this isn't an "interpreted" language.
 * However compile time evaluation looks a lot like
 * interpretation, and so that's what I went with.
 *
 * @param context
 * @return i32
 */
i32 interpret(Context *restrict context);

#endif // !EXP_CORE_INTERPRET_H
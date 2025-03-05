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
#ifndef EXP_UTILITY_PROCESS_H
#define EXP_UTILITY_PROCESS_H
#include "support/int_types.h"

/**
 * @brief fork/execvp the given file, passing args
 *
 * @warning args must have a NULL at the end
 *
 * @param file
 * @param args
 * @return i32
 */
i32 process(char const *file, char const *args[]);

#endif // !EXP_UTILITY_PROCESS_H

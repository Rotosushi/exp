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
 * @file analysis/lifetimes.h
 */

#ifndef EXP_ANALYSIS_LIFETIMES_H
#define EXP_ANALYSIS_LIFETIMES_H

#include "imr/function.h"

struct Context;
/**
 * @brief infers the lifetime of each local variable within the function.
 */
void analyze_lifetimes_of_locals(Function *function, struct Context *context);

#endif // EXP_ANALYSIS_LIFETIMES_H

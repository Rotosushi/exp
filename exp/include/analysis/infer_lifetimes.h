// Copyright (C) 2025 cade-weinberg
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

#ifndef EXP_ANALYSIS_INFER_LIFETIMES_H
#define EXP_ANALYSIS_INFER_LIFETIMES_H

#include "env/context.h"

/**
 * @brief iterates through each defined function within
 * the given context and fills in the lifetime information
 * of the SSA locals. That is, the instruction when the SSA
 * is defined, and the last instruction which it is used.
 */
i32 infer_lifetimes(Context *restrict context);

#endif // !EXP_ANALYSIS_INFER_LIFETIMES_H

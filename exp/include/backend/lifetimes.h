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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_BACKEND_LIFETIMES_H
#define EXP_BACKEND_LIFETIMES_H

#include "imr/function_body.h"

/**
 * @brief represents the lifetime of a local variable in the function.
 *
 * @note due to SSA form there is no such this as a "lifetime hole"
 * so all lifetime information is guaranteed to be contiguous. thus
 * it is safe to represent Lifetime as a (first_use, last_use) pair.
 * and this captures all relevant lifetime information.
 * hmm, we want to store lifetime information
 * "sorted by increasing last use"
 *
 *
 */
typedef struct Lifetime {
  u16 first_use;
  u16 last_use;
} Lifetime;

/**
 * @brief manages the lifetime information of all locals for a given
 * function.
 *
 * @note we don't need to dynamically grow this set because we already
 * know exactly how many SSA locals a function body uses.
 */
typedef struct Lifetimes {
  u16 count;
  Lifetime *buffer;
} Lifetimes;

Lifetimes lifetimes_create(u16 count);
void lifetimes_destroy(Lifetimes *restrict li);
Lifetime *lifetimes_at(Lifetimes *restrict li, u16 ssa);
Lifetimes lifetimes_compute(FunctionBody *restrict body);

#endif // !EXP_BACKEND_LIFETIMES_H

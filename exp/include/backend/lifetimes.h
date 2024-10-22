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

#include "env/context.h"
#include "imr/function_body.h"

/**
 * @brief represents the lifetime of a local variable in the function.
 *
 * @note due to SSA form there is no such thing as a "lifetime hole".
 * That is, all lifetime information is guaranteed to be contiguous.
 * Thus it is a complete encoding to represent the Lifetime of a given
 * SSA local as a (first_use, last_use) pair.
 * and this captures all relevant lifetime information.
 * we don't store the name of the SSA local as it is redundant.
 * We encode the name of the SSA local as the index of the given pair
 * in the array of all lifetime information.
 */
typedef struct Lifetime {
  u64 first_use;
  u64 last_use;
} Lifetime;

Lifetime lifetime_immortal(u64 Idx);
Lifetime lifetime_one_shot(u64 Idx);

/**
 * @brief manages the lifetime information of all SSA locals for a given
 * function.
 */
typedef struct Lifetimes {
  u64 count;
  u64 capacity;
  Lifetime *buffer;
} Lifetimes;

Lifetimes lifetimes_create(u64 count);
void lifetimes_destroy(Lifetimes *restrict li);
Lifetime lifetimes_at(Lifetimes *restrict li, u64 ssa);
Lifetimes lifetimes_compute(FunctionBody *restrict body,
                            Context *restrict context);
/**
 * @brief add a new lifetime to the lifetimes array
 *
 * @note the new lifetime is implicitly the lifetime of the
 * next available SSA local.
 *
 */
void lifetimes_add(Lifetimes *restrict lifetimes, Lifetime lifetime);

#endif // !EXP_BACKEND_LIFETIMES_H

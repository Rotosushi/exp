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
 * @file imr/lifetime.h
 */

#ifndef EXP_IMR_LIFETIME_H
#define EXP_IMR_LIFETIME_H

#include "utility/int_types.h"

typedef struct Lifetime {
    u32 first_use;
    u32 last_use;
} Lifetime;

Lifetime lifetime_construct(u32 first, u32 last);
Lifetime lifetime_create();
Lifetime lifetime_immortal();

#endif // EXP_IMR_LIFETIME_H

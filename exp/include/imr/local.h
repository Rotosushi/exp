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

#ifndef EXP_IMR_LOCAL_H
#define EXP_IMR_LOCAL_H

#include "imr/lifetime.h"
#include "imr/type.h"

typedef struct Local {
    u32         ssa;
    StringView  name;
    Type const *type;
    Lifetime    lifetime;
} Local;

void local_init(Local *restrict local, u32 ssa);

#endif // !EXP_IMR_LOCAL_H

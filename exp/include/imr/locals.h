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

#ifndef EXP_IMR_LOCALS_H
#define EXP_IMR_LOCALS_H

#include "imr/local.h"

typedef struct Locals {
    u32     count;
    u32     capacity;
    Local **buffer;
} Locals;

void   locals_create(Locals *restrict locals);
void   locals_destroy(Locals *restrict locals);
Local *locals_declare(Locals *restrict locals);
Local *locals_lookup(Locals *restrict locals, u32 ssa);
Local *locals_lookup_name(Locals *restrict locals, StringView name);

#endif // !EXP_IMR_LOCALS_H

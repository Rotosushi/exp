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
#ifndef EXP_IMR_LOCAL_H
#define EXP_IMR_LOCAL_H

#include "imr/lifetime.h"
#include "imr/location.h"
#include "imr/type.h"
#include "utility/string_view.h"

typedef struct Local {
    Type const *type;
    StringView label;
    Location location;
    Lifetime lifetime;
} Local;

void local_initialize(Local *local);

void local_update_type(Local *local, Type const *type);
void local_update_label(Local *local, StringView label);
void local_update_location(Local *local, Location location);
void local_update_first_use(Local *local, u64 first_use);
void local_update_last_use(Local *local, u64 last_use);

#endif // EXP_IMR_LOCAL_H
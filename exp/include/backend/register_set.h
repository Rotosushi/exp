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
#ifndef EXP_BACKEND_REGISTER_SET_H
#define EXP_BACKEND_REGISTER_SET_H

#include "backend/register.h"
#include "utility/int_types.h"

/**
 * @brief actively manages a set of registers within a function
 *  and which locals they are assigned to.
 */
typedef struct RegisterSet {
  u16 active_set;
  u16 assignments[16];
} RegisterSet;

RegisterSet register_set_create();

bool register_set_assign_next_available(RegisterSet *restrict set,
                                        Register *restrict r, u16 local);
void register_set_release(RegisterSet *restrict set, Register r);

#endif // !EXP_BACKEND_REGISTER_SET_H
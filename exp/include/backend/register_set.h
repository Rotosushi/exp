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
 * @brief a map of local to register,
 * which also tracks registers are active
 */
typedef struct RegisterSet {
  u16 bitset;
  u16 map[16];
} RegisterSet;

RegisterSet register_set_create();

/**
 * @brief preallocate register r to hold <local>
 *
 * @param rs
 * @param local
 * @param r
 */
void register_set_preallocate(RegisterSet *restrict rs, u16 local, Register r);

/**
 * @brief allocate the next free register.
 *
 * @note returns REG_NONE if all registers are filled
 *
 * @param rs
 * @param local
 * @return the allocated register
 */
Register register_set_allocate(RegisterSet *restrict rs, u16 local);

/**
 * @brief release the register holding <local>
 *
 * @param rs
 * @param r
 * @return the released register
 */
Register register_set_release(RegisterSet *restrict rs, u16 local);

#endif // !EXP_BACKEND_REGISTER_SET_H
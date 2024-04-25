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
 * @brief a bitset of which registers are active
 */
typedef u16 RegisterSet;

/**
 * @brief preallocate register r
 *
 * @param rs
 * @param r
 */
void register_set_preallocate(RegisterSet *restrict rs, Register r);

/**
 * @brief allocate the next free register.
 *
 * @note returns REG_NONE if all registers are filled
 *
 * @param rs
 * @return Register
 */
Register register_set_next_available(RegisterSet *restrict rs);

/**
 * @brief release register <r>
 *
 * @param rs
 * @param r
 */
void register_set_release(RegisterSet *restrict rs, Register r);

#endif // !EXP_BACKEND_REGISTER_SET_H
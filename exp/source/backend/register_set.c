/**
 * Copyright (C) 2024 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "backend/register_set.h"

#define SET_BIT(RS, r) ((RS) |= ((RegisterSet)1 << (RegisterSet)r))

#define CLR_BIT(RS, r)                                                         \
  ((RS) &= (RegisterSet) ~((RegisterSet)1 << (RegisterSet)r))

#define CHK_BIT(RS, r) (((RS) >> (RegisterSet)r) & (RegisterSet)1)

void register_set_preallocate(RegisterSet *restrict rs, Register r) {
  SET_BIT(*rs, r);
}

Register register_set_next_available(RegisterSet *restrict rs) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(*rs, i)) {
      return (Register)i;
    }
  }
  return REG_NONE;
}

void register_set_release(RegisterSet *restrict rs, Register r) {
  CLR_BIT(*rs, r);
}

#undef SET_BIT
#undef CLR_BIT
#undef CHK_BIT
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
#include <assert.h>

#include "backend/register_set.h"

#define SET_BIT(B, r) ((B) |= ((u16)1 << (u16)r))

#define CLR_BIT(B, r) ((B) &= (u16) ~((u16)1 << (u16)r))

#define CHK_BIT(B, r) (((B) >> (u16)r) & (u16)1)

RegisterSet register_set_create() {
  RegisterSet rs = {.bitset = 0};
  return rs;
}

void register_set_preallocate(RegisterSet *restrict rs, u16 local, Register r) {
  assert(r != REG_NONE);
  SET_BIT(rs->bitset, r);
  rs->map[r] = local;
}

Register register_set_allocate(RegisterSet *restrict rs, u16 local) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(rs->bitset, i)) {
      SET_BIT(rs->bitset, i);
      Register r = i;
      rs->map[r] = local;
      return r;
    }
  }
  return REG_NONE;
}

Register register_set_release(RegisterSet *restrict rs, u16 local) {
  for (u8 i = 0; i < 16; ++i) {
    if (rs->map[i] == local) {
      CLR_BIT(rs->bitset, i);
      return i;
    }
  }
  return REG_NONE;
}

#undef SET_BIT
#undef CLR_BIT
#undef CHK_BIT
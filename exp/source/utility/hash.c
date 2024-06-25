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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "utility/hash.h"

// non-crypto hash algorithm based on djb2
// https://stackoverflow.com/questions/7666509/hash-function-for-string
// specifically this answer https://stackoverflow.com/a/69812981
u64 hash_cstring(char const *restrict string, u64 length) {
  // generated randomly using: https://asecuritysite.com/encryption/nprimes?y=64
  // no testing has been done to check if this prime is "good"
#define LARGE_PRIME 11931085111904720063ul

  u64 hash = 5381;
  for (u64 i = 0; i < length; ++i)
    hash = (LARGE_PRIME * hash) + (u8)(string[i]);
  return hash;

#undef LARGE_PRIME
}
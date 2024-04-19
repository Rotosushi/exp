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
#include "utility/alloc.h"
#include "utility/panic.h"

void *allocate(u64 size) {
  void *result = malloc(size);
  if (result == NULL) {
    PANIC_ERRNO("malloc failed");
  }
  return result;
}

void *reallocate(void *ptr, u64 size) {
  void *result = realloc(ptr, size);
  if (result == NULL) {
    PANIC_ERRNO("reallocate failed.");
  }
  return result;
}

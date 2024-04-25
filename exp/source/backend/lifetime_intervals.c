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
#include <string.h>

#include "backend/lifetime_intervals.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

LifetimeIntervals lifetime_intervals_create() {
  LifetimeIntervals li;
  li.size     = 0;
  li.capacity = 0;
  li.buffer   = NULL;
  return li;
}

void lifetime_intervals_destroy(LifetimeIntervals *restrict li) {
  assert(li != NULL);
  li->size     = 0;
  li->capacity = 0;
  free(li->buffer);
  li->buffer = NULL;
}

static bool lifetime_intervals_full(LifetimeIntervals *restrict li) {
  return (li->size + 1) >= li->capacity;
}

static void lifetime_intervals_grow(LifetimeIntervals *restrict li) {
  Growth g     = array_growth_u16(li->capacity, sizeof(Interval));
  li->buffer   = reallocate(li->buffer, g.alloc_size);
  li->capacity = (u16)g.new_capacity;
}

void lifetime_intervals_insert_sorted(LifetimeIntervals *restrict li,
                                      Interval interval) {
  assert(li != NULL);

  if (lifetime_intervals_full(li)) {
    lifetime_intervals_grow(li);
  }

  // find the first interval that begins later than the new interval
  // and insert just before it.
  u16 i = 0;
  for (; i < li->size; ++i) {
    if (li->buffer[i].first_use > interval.first_use) {
      break;
    }
  }

  // shift all intervals after i forward one location
  for (u16 j = li->size; j > i; --j) {
    li->buffer[j] = li->buffer[j - 1];
  }

  li->buffer[i] = interval;
  li->size += 1;
}

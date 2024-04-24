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
#include <stdlib.h>

#include "backend/lifetime_intervals.h"
#include "utility/numeric_conversions.h"

[[maybe_unused]] static void print_li(LifetimeIntervals *restrict li,
                                      FILE *file) {
  for (u16 i = 0; i < li->size; ++i) {
    Interval lifetime = li->buffer[i];
    file_write("[", file);
    print_u64(lifetime.local, RADIX_DECIMAL, file);
    file_write(", ", file);
    print_u64(lifetime.begin, RADIX_DECIMAL, file);
    file_write(", ", file);
    print_u64(lifetime.end, RADIX_DECIMAL, file);
    file_write("]", file);
  }
  file_write("\n", file);
}

static bool intervals_sorted(LifetimeIntervals *restrict li) {
  for (u16 i = 0; i < (li->size - 1); ++i) {
    if (li->buffer[i].begin > li->buffer[i + 1].begin) {
      return 0;
    }
  }
  return 1;
}

static Interval create_interval() {
  Interval i = {.local = (u16)(rand() & u16_MAX),
                .begin = (u16)(rand() & u16_MAX),
                .end   = (u16)(rand() & u16_MAX)};
  return i;
}

int lifetime_intervals_tests([[maybe_unused]] int argc,
                             [[maybe_unused]] char **argv) {
  LifetimeIntervals li = lifetime_intervals_create();

  lifetime_intervals_insert_sorted(&li, create_interval());
  // print_li(&li, stdout);
  lifetime_intervals_insert_sorted(&li, create_interval());
  // print_li(&li, stdout);
  lifetime_intervals_insert_sorted(&li, create_interval());
  // print_li(&li, stdout);
  lifetime_intervals_insert_sorted(&li, create_interval());
  // print_li(&li, stdout);

  bool failure = !intervals_sorted(&li);

  lifetime_intervals_destroy(&li);
  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}
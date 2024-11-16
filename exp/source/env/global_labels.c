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
#include <assert.h>

#include "env/global_labels.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

GlobalLabels global_labels_create() {
  GlobalLabels symbols = {.count = 0, .capacity = 0, .buffer = NULL};
  return symbols;
}

void global_labels_destroy(GlobalLabels *restrict symbols) {
  assert(symbols != NULL);
  deallocate(symbols->buffer);
  symbols->buffer   = NULL;
  symbols->count    = 0;
  symbols->capacity = 0;
}

static bool global_labels_full(GlobalLabels *restrict symbols) {
  return (symbols->count + 1) >= symbols->capacity;
}

static void global_labels_grow(GlobalLabels *restrict symbols) {
  Growth g          = array_growth_u16(symbols->capacity, sizeof(StringView));
  symbols->buffer   = reallocate(symbols->buffer, g.alloc_size);
  symbols->capacity = (u16)g.new_capacity;
}

u16 global_labels_insert(GlobalLabels *restrict symbols, StringView symbol) {
  assert(symbols != NULL);

  if (global_labels_full(symbols)) { global_labels_grow(symbols); }

  for (u64 i = 0; i < symbols->count; ++i) {
    assert(i < u16_MAX);
    StringView s = symbols->buffer[i];
    if (string_view_equality(s, symbol)) { return (u16)i; }
  }

  u16 index              = symbols->count;
  symbols->buffer[index] = symbol;
  symbols->count += 1;
  return index;
}

StringView global_labels_at(GlobalLabels *restrict symbols, u16 index) {
  assert(symbols != NULL);
  assert(index < symbols->count);
  return symbols->buffer[index];
}

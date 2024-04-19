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
#include <stdlib.h>

#include "env/module.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

Module module_create() {
  Module m;
  m.capacity = 0;
  m.size     = 0;
  m.list     = NULL;
  return m;
}

void module_destroy(Module *restrict m) {
  assert(m != NULL);

  for (u64 i = 0; i < m->size; ++i) {
    ast_destroy(m->list[i]);
  }

  m->capacity = 0;
  m->size     = 0;
  free(m->list);
  m->list = NULL;
}

static bool module_full(Module *restrict m) {
  return m->capacity <= m->size + 1;
}

static void module_grow(Module *restrict m) {
  Growth g    = array_growth(m->capacity, sizeof(Ast *));
  m->list     = reallocate(m->list, g.alloc_size);
  m->capacity = g.new_capacity;
}

void module_add(Module *restrict m, Ast *restrict a) {
  assert(m != NULL);
  assert(a != NULL);

  if (module_full(m)) {
    module_grow(m);
  }

  m->list[m->size] = a;
  m->size += 1;
}

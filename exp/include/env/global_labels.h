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
#ifndef EXP_ENV_GLOBAL_SYMBOLS_H
#define EXP_ENV_GLOBAL_SYMBOLS_H
#include "utility/string_view.h"

typedef struct GlobalLabels {
  u16 size;
  u16 capacity;
  StringView *buffer;
} GlobalLabels;

GlobalLabels global_labels_create();
void global_labels_destroy(GlobalLabels *restrict symbols);

u16 global_labels_insert(GlobalLabels *restrict symbols, StringView symbol);
StringView global_labels_at(GlobalLabels *restrict symbols, u16 idx);

#endif // !EXP_ENV_GLOBAL_SYMBOLS_H
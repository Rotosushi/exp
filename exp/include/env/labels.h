// Copyright (C) 2025 Cade Weinberg
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

/**
 * @file env/global_symbols.h
 */

#ifndef EXP_ENV_GLOBAL_SYMBOLS_H
#define EXP_ENV_GLOBAL_SYMBOLS_H

#include "utility/string_view.h"

typedef struct GlobalLabels {
    u32 count;
    u32 capacity;
    StringView *buffer;
} Labels;

void labels_initialize(Labels *labels);
void labels_terminate(Labels *labels);

u32 labels_insert(Labels *labels, StringView label);
StringView labels_at(Labels *labels, u32 index);

#endif // !EXP_ENV_GLOBAL_SYMBOLS_H

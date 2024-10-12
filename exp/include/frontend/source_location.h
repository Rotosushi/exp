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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_ENV_SOURCE_LOCATION_H
#define EXP_ENV_SOURCE_LOCATION_H

#include "utility/string_view.h"

typedef struct SourceLocation {
  StringView file;
  u64 line;
  u64 column;
} SourceLocation;

SourceLocation source_location_create();
SourceLocation source_location(StringView file, u64 line, u64 column);

bool source_location_equality(SourceLocation A, SourceLocation B);

void print_source_location(struct String *restrict out,
                           SourceLocation location);

#endif // EXP_ENV_SOURCE_LOCATION_H

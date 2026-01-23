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

#ifndef EXP_SUPPORT_SOURCE_LOCATION_H
#define EXP_SUPPORT_SOURCE_LOCATION_H

#include "support/string_view.h"

typedef struct SourceLocation {
    StringView file;
    u64        line;
    u64        column;
} SourceLocation;

void source_location_initialize(SourceLocation *restrict source_location,
                                StringView file,
                                u64        line,
                                u64        column);

#define SOURCE_LOCATION_CURRENT()                                              \
    ((SourceLocation){.file = SV(__FILE__), .line = __LINE__})

#endif // !EXP_SUPPORT_SOURCE_LOCATION_H

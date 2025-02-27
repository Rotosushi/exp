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
#ifndef EXP_UTILITY_PANIC_H
#define EXP_UTILITY_PANIC_H

#include "utility/string_view.h"

[[noreturn]] void panic(StringView msg, const char *file, i32 line);
[[noreturn]] void panic_errno(StringView msg, const char *file, i32 line);

#define PANIC(msg)       panic(SV(msg), __FILE__, __LINE__)
#define PANIC_ERRNO(msg) panic_errno(SV(msg), __FILE__, __LINE__)

#endif // !EXP_UTILITY_PANIC_H

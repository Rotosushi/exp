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
 * @file support/io.h
 * @brief wrappers around C stdio functions
 * that panic on error.
 */

#ifndef EXP_SUPPORT_IO_H
#define EXP_SUPPORT_IO_H

#include <stdio.h>

#include "support/scalar.h"
#include "support/string.h"
#include "support/string_view.h"

FILE *file_open(char const *restrict path, char const *restrict modes);
void  file_close(FILE *restrict file);
void  file_remove(char const *restrict path);
u64   file_length(FILE *restrict stream);
void  file_write(StringView view, FILE *restrict stream);
void  file_write_i64(i64 value, FILE *restrict stream);
void  file_write_u64(u64 value, FILE *restrict stream);
u64   file_read(char *restrict buffer, u64 size, FILE *restrict stream);

void file_read_all(String *restrict string, StringView path);
void file_write_all(String *restrict string, StringView path);

#endif // !EXP_SUPPORT_IO_H

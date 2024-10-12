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
#ifndef EXP_UTILITY_IO_H
#define EXP_UTILITY_IO_H

#include <stdio.h>

#include "utility/string_view.h"

FILE *file_open(StringView path, StringView modes);

void file_close(FILE *restrict file);

void file_remove(StringView path);

/**
 * @brief write <buffer> to <stream>
 *
 * @note if the write fails, an error is printed to stderr
 * and exit is called
 *
 * @param buffer the buffer to write
 * @param length the length of the buffer
 * @param stream the stream to write to
 * @return u64 the number of chars written
 */
void file_write(FILE *restrict out, StringView buffer);

void file_write_i64(FILE *restrict out, i64 value);

void file_write_u64(FILE *restrict out, u64 value);

/**
 * @brief read <length> chars into <buffer> from <stream>
 *
 * @param buffer the buffer to store chars into
 * @param length the number of chars to read
 * @param file the stream to read from.
 * @return u64 the number of chars actually read.
 */
u64 file_read(FILE *restrict file, char *buffer, u64 length);

u64 file_length(FILE *restrict file);

#endif // !EXP_UTILITY_IO_H

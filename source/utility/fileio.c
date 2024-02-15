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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>

#include "utility/fileio.h"

size_t file_write(const char *restrict buffer, size_t length,
                  FILE *restrict stream) {
  size_t bytes_written = 0;
  int code = 0;
  for (size_t i = 0; (code != EOF) && (i < length); ++i) {
    code = fputc(buffer[i], stream);
    bytes_written++;
  }

  if ((code == EOF) && (ferror(stream))) {
    perror("putc failed");
    exit(EXIT_FAILURE);
  }

  return bytes_written;
}

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

#include "filesystem/io.h"

void file_write(const char *restrict buffer, [[maybe_unused]] size_t length,
                FILE *restrict stream) {
  int code = fputs(buffer, stream);
  if ((code == EOF) && (ferror(stream))) {
    perror("fputs failed");
    exit(EXIT_FAILURE);
  }
}

size_t file_read(char *buffer, size_t length, FILE *restrict stream) {
  char *result = fgets(buffer, (int)length, stream);
  if (result == NULL) {
    perror("fgets failed");
    exit(EXIT_FAILURE);
  }

  return (size_t)(result - buffer);
}

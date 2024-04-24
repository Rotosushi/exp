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
#include <assert.h>
#include <stdlib.h>

#include "utility/config.h"
#include "utility/io.h"
#include "utility/panic.h"

FILE *file_open(char const *restrict path, char const *restrict modes) {
  assert(path != NULL);
  assert(modes != NULL);
  FILE *file = fopen(path, modes);
  if (file == NULL) {
    PANIC_ERRNO("fopen failed");
  }
  return file;
}

void file_close(FILE *restrict file) {
  assert(file != NULL);
  if (fclose(file) == EOF) {
    PANIC_ERRNO("fclose failed");
  }
}

void file_remove(char const *restrict path) {
  if (remove(path)) {
    PANIC_ERRNO("remove failed");
  }
}

void file_write(const char *restrict buffer, FILE *restrict stream) {
  i32 code = fputs(buffer, stream);
  if ((code == EOF) && (ferror(stream))) {
    PANIC_ERRNO("fputs failed");
  }
}

u64 file_read(char *buffer, u64 length, FILE *restrict stream) {
  char *result = fgets(buffer, (i32)length, stream);
  if (result == NULL) {
    PANIC_ERRNO("fgets failed");
  }

  return (u64)(result - buffer);
}

#if defined(EXP_HOST_OS_LINUX)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

u64 file_length(FILE *restrict file) {
  i32 fd = fileno(file);

  struct stat info;
  if (fstat(fd, &info) != 0) {
    PANIC_ERRNO("fstat failed");
  }

  return (u64)info.st_size;
}

#else
u64 file_length(FILE *restrict file) {
  if (fseek(file, 0L, SEEK_END) != 0) {
    PANIC_ERRNO("fseek failed");
  }

  i64 size = ftell(file);
  if (size == -1L) {
    PANIC_ERRNO("ftell failed");
  }

  rewind(file);
  return (u64)size;
}
#endif

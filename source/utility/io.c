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
  int code = fputs(buffer, stream);
  if ((code == EOF) && (ferror(stream))) {
    PANIC_ERRNO("fputs failed");
  }
}

size_t file_read(char *buffer, size_t length, FILE *restrict stream) {
  char *result = fgets(buffer, (int)length, stream);
  if (result == NULL) {
    PANIC_ERRNO("fgets failed");
  }

  return (size_t)(result - buffer);
}

#if defined(EXP_HOST_OS_LINUX)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

size_t file_length(FILE *restrict file) {
  int fd = fileno(file);

  struct stat info;
  if (fstat(fd, &info) != 0) {
    PANIC_ERRNO("fstat failed");
  }

  return (size_t)info.st_size;
}

#else
size_t file_length(FILE *restrict file) {
  if (fseek(file, 0L, SEEK_END) != 0) {
    PANIC_ERRNO("fseek failed");
  }

  long size = ftell(file);
  if (size == -1L) {
    PANIC_ERRNO("ftell failed");
  }

  rewind(file);
  return (size_t)size;
}
#endif

String file_readall(FILE *restrict stream) {
  assert(stream != NULL);

  String result = string_create();
  string_resize(&result, file_length(stream));
#define BUFSZ 1024
  static char buffer[BUFSZ];
  while (1) {
    char *p = fgets(buffer, BUFSZ, stream);
    if ((p == NULL)) {
      if (feof(stream)) {
        break;
      } else if (ferror(stream)) {
        fclose(stream);
        PANIC_ERRNO("fgets failed");
      }
    }

    string_append(&result, buffer);

    if (feof(stream)) {
      break;
    }
  }
#undef BUFSZ

  return result;
}

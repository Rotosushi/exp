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
#include <ctype.h>
#include <math.h>

#include "utility/numeric_conversions.h"
#include "utility/panic.h"

[[maybe_unused]] static bool valid_radix(Radix r) {
  return (r == RADIX_BINARY) || (r == RADIX_OCTAL) || (r == RADIX_DECIMAL) ||
         (r == RADIX_HEXADECIMAL);
}

u64 i64_safe_strlen(i64 value, Radix radix) {
  u64 result = 0;

  if ((value == 0) || (value == 1)) {
    // "0" or "1"
    result += 1;
    return result;
  }

  if (value < 0) {
    // negative sign "-"
    result += 1;
    if (value == -1) {
      // "1"
      result += 1;
      return result;
    }

    double absolute   = fabs((double)value);
    u64 number_length = (u64)(ceil(log(absolute) / log((double)radix)));
    if (__builtin_add_overflow(result, number_length, &result)) {
      PANIC("value exceeds u64");
    }
  } else {
    u64 number_length = (u64)(ceil(log((double)value) / log((double)radix)));
    if (__builtin_add_overflow(result, number_length, &result)) {
      PANIC("value exceeds u64");
    }
  }
  return result;
}

u64 u64_safe_strlen(u64 value, Radix radix) {
  u64 result = 0;

  if ((value == 0) || (value == 1)) {
    // "0" or "1"
    result += 1;
    return result;
  }

  u64 number_length = (u64)(ceil(log((double)value) / log((double)radix)));
  if (__builtin_add_overflow(result, number_length, &result)) {
    PANIC("value exceeds u64");
  }
  return result;
}

char *i64_to_str(i64 value, char *restrict buffer, Radix radix) {
  assert(valid_radix(radix));
  assert(buffer != NULL);

  i64 tmp_value;
  char *ptr1, *ptr2;
  char mapping[] =
      "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

  // convert the number
  ptr1 = buffer;
  do {
    tmp_value = value;
    value /= radix;
    *ptr1++ = mapping[35 + (tmp_value - value * radix)];
  } while (value);

  // append the sign
  if (tmp_value < 0) {
    *ptr1++ = '-';
  }
  // null terminate
  ptr2    = buffer;
  buffer  = ptr1;
  *ptr1-- = '\0';

  // reverse the string
  while (ptr2 < ptr1) {
    char tmp = *ptr1;
    *ptr1--  = *ptr2;
    *ptr2++  = tmp;
  }

  return buffer;
}

void print_i64(i64 value, Radix radix, FILE *file) {
  char buf[i64_safe_strlen(value, radix) + 1];
  if (i64_to_str(value, buf, radix) == NULL) {
    PANIC("conversion failed");
  }
  fputs(buf, file);
}

char *u64_to_str(u64 value, char *restrict buffer, Radix radix) {
  assert(valid_radix(radix));
  assert(buffer != NULL);

  char *ptr1, *ptr2;
  char mapping[] =
      "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

  // convert the number
  ptr1 = buffer;
  do {
    u64 tmp = value;
    value /= radix;
    *ptr1++ = mapping[35 + (tmp - value * radix)];
  } while (value);

  // append the null terminator
  ptr2    = buffer;
  buffer  = ptr1;
  *ptr1-- = '\0';

  // reverse the string
  while (ptr2 < ptr1) {
    char tmp = *ptr1;
    *ptr1--  = *ptr2;
    *ptr2++  = tmp;
  }

  return buffer;
}

void print_u64(u64 value, Radix radix, FILE *file) {
  char buf[u64_safe_strlen(value, radix) + 1];
  if (u64_to_str(value, buf, radix) == NULL) {
    PANIC("conversion failed");
  }
  fputs(buf, file);
}

// static u64 char_value(char c) {
//   switch (c) {
//   case '0':
//   case '1':
//   case '2':
//   case '3':
//   case '4':
//   case '5':
//   case '6':
//   case '7':
//   case '8':
//   case '9':
//     return c - '0';

//   default:
//     unreachable();
//   }
// }

static u64 base10_stou64(char const *restrict buffer, u64 length) {
  u64 result = 0;
  u64 i      = 0;

  while (i < length && isdigit(buffer[i])) {
    if (__builtin_mul_overflow(result, 10, &result)) {
      PANIC("string out of range of u64");
    }

    if (__builtin_add_overflow(result, (buffer[i] - '0'), &result)) {
      PANIC("string out of range of u64");
    }

    ++i;
  }

  return result;
}

static i64 base10_stoi64(char const *restrict buffer, u64 length) {
  char const *cursor = buffer;
  i64 sign           = 1;
  if (*cursor == '-') {
    sign = -1;
    ++cursor;
  }

  u64 val = base10_stou64(cursor, length);
  if ((val == ((u64)i64_MAX + 1)) && (sign == -1)) {
    return i64_MIN;
  } else if (val > i64_MAX) {
    PANIC("string out of range of i64");
  }

  // since we know val <= i64_MAX it is always safe to
  // cast val to an i64.
  return (i64)val * sign;
}

i64 str_to_i64(char const *restrict buffer, u64 length, Radix radix) {
  assert(valid_radix(radix));
  assert(buffer != NULL);

  switch (radix) {
  case RADIX_DECIMAL:
    return base10_stoi64(buffer, length);

  case RADIX_BINARY:
  case RADIX_OCTAL:
  case RADIX_HEXADECIMAL:
  default:
    PANIC("unsupported radix");
  }

  return 0;
}

u64 str_to_u64(char const *restrict buffer, u64 length, Radix radix) {
  assert(valid_radix(radix));
  assert(buffer != NULL);

  switch (radix) {
  case RADIX_DECIMAL:
    return base10_stou64(buffer, length);

  case RADIX_BINARY:
  case RADIX_OCTAL:
  case RADIX_HEXADECIMAL:
  default:
    PANIC("unsupported radix");
  }
}
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
#include <math.h>

#include "utility/numbers_to_string.h"
#include "utility/panic.h"

size_t intmax_safe_strlen(intmax_t value, Radix radix) {
  size_t result = 0;

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

    double absolute = fabs((double)value);
    size_t number_length = (size_t)(ceil(log(absolute) / log((double)radix)));
    if (__builtin_add_overflow(result, number_length, &result)) {
      panic("value exceeds size_t");
    }
    return result;
  } else {
    size_t number_length =
        (size_t)(ceil(log((double)value) / log((double)radix)));
    if (__builtin_add_overflow(result, number_length, &result)) {
      panic("value exceeds size_t");
    }
    return result;
  }
}

size_t uintmax_safe_strlen(uintmax_t value, Radix radix) {
  size_t result = 0;

  if ((value == 0) || (value == 1)) {
    // "0" or "1"
    result += 1;
    return result;
  }

  size_t number_length =
      (size_t)(ceil(log((double)value) / log((double)radix)));
  if (__builtin_add_overflow(result, number_length, &result)) {
    panic("value exceeds size_t");
  }
  return result;
}

char *intmax_to_str(intmax_t value, char *restrict buffer, Radix radix) {
  assert((radix == RADIX_BINARY) || (radix == RADIX_OCTAL) ||
         (radix == RADIX_DECIMAL) || (radix == RADIX_HEXADECIMAL));

  intmax_t tmp_value;
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
  ptr2 = buffer;
  buffer = ptr1;
  *ptr1-- = '\0';

  // reverse the string
  while (ptr2 < ptr1) {
    char tmp = *ptr1;
    *ptr1-- = *ptr2;
    *ptr2++ = tmp;
  }

  return buffer;
}

String intmax_to_string(intmax_t value, Radix radix) {
  String str = string_create();

  string_resize(&str, intmax_safe_strlen(value, radix));

  char *end = intmax_to_str(value, str.buffer, radix);

  str.length = (size_t)(end - str.buffer);

  return str;
}

char *uintmax_to_str(uintmax_t value, char *restrict buffer, Radix radix) {
  assert((radix == RADIX_BINARY) || (radix == RADIX_OCTAL) ||
         (radix == RADIX_DECIMAL) || (radix == RADIX_HEXADECIMAL));

  char *ptr1, *ptr2;
  char mapping[] =
      "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

  // convert the number
  ptr1 = buffer;
  do {
    uintmax_t tmp = value;
    value /= radix;
    *ptr1++ = mapping[35 + (tmp - value * radix)];
  } while (value);

  // append the null terminator
  ptr2 = buffer;
  buffer = ptr1;
  *ptr1-- = '\0';

  // reverse the string
  while (ptr2 < ptr1) {
    char tmp = *ptr1;
    *ptr1-- = *ptr2;
    *ptr2++ = tmp;
  }

  return buffer;
}

String uintmax_to_string(uintmax_t value, Radix radix) {
  String str = string_create();

  string_resize(&str, uintmax_safe_strlen(value, radix));

  char *end = uintmax_to_str(value, str.buffer, radix);

  str.length = (size_t)(end - str.buffer);

  return str;
}
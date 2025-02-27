/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include <string.h>

#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/minmax.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"
#include "utility/string.h"

void string_initialize(String *string) {
    EXP_ASSERT(string != nullptr);
    string->length    = 0;
    string->capacity  = sizeof(char *);
    string->buffer[0] = '\0';
}

static bool string_is_small(String const *string) {
    EXP_ASSERT(string != nullptr);
    return string->capacity <= sizeof(char *);
}

void string_terminate(String *string) {
    EXP_ASSERT(string != nullptr);
    if (!string_is_small(string)) {
        deallocate(string->ptr);
        string->ptr = nullptr;
    } else {
        string->buffer[0] = '\0';
    }
    string->length   = 0;
    string->capacity = sizeof(char *);
}

StringView string_to_view(String const *string) {
    EXP_ASSERT(string != nullptr);
    StringView view = string_view_create();
    view = string_view_from_str(string_to_cstring(string), string->length);
    return view;
}

char const *string_to_cstring(String const *string) {
    EXP_ASSERT(string != nullptr);
    if (string_is_small(string)) {
        return string->buffer;
    } else {
        return string->ptr;
    }
}

char *string_data(String *string) {
    EXP_ASSERT(string != nullptr);
    if (string_is_small(string)) {
        return string->buffer;
    } else {
        return string->ptr;
    }
}

void string_assign(String *string, StringView view) {
    EXP_ASSERT(string != nullptr);
    string_terminate(string);
    if ((view.length == u64_MAX) || (view.length + 1 == u64_MAX)) {
        PANIC("cannot allocate more than u64_MAX.");
    }
    string->length   = view.length;
    string->capacity = max_u64(view.length + 1, string->capacity);

    if (string->length < sizeof(char *)) {
        memcpy(string->buffer, view.ptr, string->length);
        string->buffer[string->length] = '\0';
    } else {
        string->ptr = callocate(string->capacity, sizeof(char));
        memcpy(string->ptr, view.ptr, string->length);
    }
}

void string_assign_string(String *target, String const *source) {
    EXP_ASSERT(target != nullptr);
    EXP_ASSERT(source != nullptr);
    string_terminate(target);
    string_assign(target, string_to_view(source));
}

void string_from_view(String *string, StringView view) {
    EXP_ASSERT(string != nullptr);
    string_initialize(string);
    string_assign(string, view);
}

void string_from_cstring(String *string, char const *cstring) {
    EXP_ASSERT(string != nullptr);
    EXP_ASSERT(cstring != nullptr);
    string_initialize(string);
    string_from_view(string, string_view_from_cstring(cstring));
}

/*
void string_from_file(String *string, FILE *file) {
    EXP_ASSERT(string != nullptr);
    EXP_ASSERT(file != nullptr);
    string_initialize(string);
    u64 flen = file_length(file);
    string_resize(string, flen);
    if (flen < sizeof(char *)) {
        file_read(string->buffer, flen, file);
    } else {
        file_read(string->ptr, flen, file);
    }

    string->length = flen;
}
*/

bool string_empty(String const *string) {
    EXP_ASSERT(string != nullptr);
    return string->length == 0;
}

bool string_equality(String const *string, StringView view) {
    EXP_ASSERT(string != nullptr);
    if (string->length != view.length) { return 0; }

    if (string_is_small(string)) {
        return strncmp(string->buffer, view.ptr, view.length) == 0;
    }

    return strncmp(string->ptr, view.ptr, view.length) == 0;
}

void string_resize(String *string, u64 capacity) {
    EXP_ASSERT(string != nullptr);
    if (string_is_small(string)) {
        if (capacity >= sizeof(char *)) {
            char *buf        = callocate(capacity, sizeof(char));
            string->capacity = capacity;
            memcpy(buf, string->buffer, string->length);
            string->ptr = buf;
        } // else the small capacity can hold the given capacity
        return;
    }

    string->ptr                 = reallocate(string->ptr, capacity);
    string->capacity            = capacity;
    string->ptr[string->length] = '\0';
}

void string_append(String *string, StringView view) {
    if (view.length == 0) { return; }

    if ((string->length + view.length) >= string->capacity) {
        Growth64 g = array_growth_u64(string->capacity + view.length,
                                      sizeof(*string->ptr));
        string_resize(string, g.new_capacity);
    }

    if (string_is_small(string)) {
        memcpy(string->buffer + string->length, view.ptr, view.length);
        string->length += view.length;
        string->buffer[string->length] = '\0';
    } else {
        memcpy(string->ptr + string->length, view.ptr, view.length);
        string->length += view.length;
        string->ptr[string->length] = '\0';
    }
}

void string_append_string(String *target, String const *source) {
    EXP_ASSERT(target != nullptr);
    EXP_ASSERT(source != nullptr);
    string_append(target, string_to_view(source));
}

void string_append_i64(String *string, i64 value) {
    u64 len = i64_safe_strlen(value);
    char buf[len + 1];
    char *r = i64_to_str(value, buf);
    if (r == nullptr) { PANIC("conversion failed"); }
    buf[len] = '\0';
    string_append(string, string_view_from_str(buf, len));
}

void string_append_u64(String *string, u64 value) {
    u64 len = u64_safe_strlen(value);
    char buf[len + 1];
    char *r = u64_to_str(value, buf);
    if (r == nullptr) { PANIC("conversion failed"); }
    buf[len] = '\0';
    string_append(string, string_view_from_str(buf, len));
}

/*
  essentially can be broken into these cases

  1 offset == 0, length == str->length

  2 offset == 0, length < str->length

  3 offset > 0, (offset + length) == str->length

  4 offset > 0, (offset + length) < str->length
*/
void string_erase(String *string, u64 offset, u64 length) {
    EXP_ASSERT(string != nullptr);
    EXP_ASSERT(offset <= string->length);
    EXP_ASSERT((offset + length) <= string->length);

    if ((offset == 0) && (length == string->length)) {
        // erase the entire buffer
        if (string_is_small(string)) {
            string->buffer[0] = '\0';
        } else {
            string->ptr[0] = '\0';
        }
        string->length = 0;
        return;
    }

    // erase <length> characters starting from <str->buffer + offset>
    if (string_is_small(string)) {
        char *pos       = string->buffer + offset;
        char *rest      = pos + length;
        u64 rest_length = (u64)((string->buffer + string->length) - rest);
        memmove(pos, rest, rest_length);
        u64 new_length             = offset + rest_length;
        string->buffer[new_length] = '\0';
        string->length             = new_length;
    } else {
        char *pos       = string->ptr + offset;
        char *rest      = pos + length;
        u64 rest_length = (u64)((string->ptr + string->length) - rest);
        memmove(pos, rest, rest_length);
        u64 new_length          = offset + rest_length;
        string->ptr[new_length] = '\0';
        string->length          = new_length;
    }
}

/*
  there are four cases that need to be considered.

  1 offset == 0, length < str->length

  2 offset == 0, length >= str->length

  3 offset > 0, offset + length < str->length

  4 offset > 0, offset + length >= str->length

  case 1, we can just write the data into the existing buffer
    new_length == existing length
  case 3, we can just write the data into the existing buffer
    new_length == existing length

  case 2, we have to resize the existing buffer, then we can write

  case 4, we have to resize the existing buffer, then we can write
*/
void string_insert(String *string, u64 offset, StringView view) {
    EXP_ASSERT(string != nullptr);
    EXP_ASSERT(offset <= string->length);

    if ((offset + view.length) >= string->capacity) {
        string_resize(string, (offset + view.length) + string->length);
        u64 added_length = (offset + view.length) - string->length;
        string->length += added_length;
    }

    if (string_is_small(string)) {
        memcpy(string->buffer + offset, view.ptr, view.length);
        string->buffer[offset + view.length] = '\0';
    } else {
        memcpy(string->ptr + offset, view.ptr, view.length);
        string->ptr[offset + view.length] = '\0';
    }
}

StringView string_extension(String const *string) {
    EXP_ASSERT(string != nullptr);
    u64 cursor         = string->length;
    char const *buffer = string_to_cstring(string);
    while ((cursor != 0) && (buffer[cursor] != '/')) {
        --cursor;
    }

    if (buffer[cursor] == '/') { ++cursor; }

    if (buffer[cursor] == '.') { ++cursor; }

    u64 target = cursor;
    while ((cursor < string->length)) {
        if (buffer[cursor] == '.') { target = cursor; }
        ++cursor;
    }

    return string_view_from_str(buffer + target, cursor - target);
}

void string_replace_extension(String *string, StringView extension) {
    EXP_ASSERT(string != nullptr);
    // the string is something like
    // /some/kind/of/file.txt
    // or
    // /some/kind/of/.file.txt
    // or
    // /some/kind/of/.file
    // of
    // /some/kind/of/file.with.multiple.extensions

    // search for the last '/' in the string
    u64 length   = string->length;
    u64 cursor   = length;
    char *buffer = string_is_small(string) ? string->buffer : string->ptr;
    while ((cursor != 0) && (buffer[cursor] != '/')) {
        --cursor;
    }

    // set the cursor to the first char in the filename
    if (buffer[cursor] == '/') { ++cursor; }

    // if the first character in the path is '.' ignore it
    if (buffer[cursor] == '.') { ++cursor; }

    // find the beginning of the final extension
    // or the end of the filename
    u64 target = cursor;
    while ((cursor < length)) {
        if (buffer[cursor] == '.') { target = cursor; }
        ++cursor;
    }

    if (extension.length == 0) {
        // remove the extension
        buffer[target] = '\0';
        string->length = target;
    } else {
        if (extension.ptr[0] != '.') {
            if (buffer[target] != '.') {
                string_append(string, SV("."));
            } else {
                ++target;
            }
        }
        // insert the given extension.
        string_insert(string, target, extension);
    }
}

// void print_string(String * s, FILE * file) {
//   file_write(s->buffer, file);
// }

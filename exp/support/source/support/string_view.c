// SPDX-License-Identifier: GPL-3.0-or-later

#include <assert.h>
#include <string.h>

#include "support/string_view.h"

bool string_view_eq(StringView a, StringView b) {
    if (a.length != b.length) { return false; }
    return strncmp(a.data, b.data, a.length) == 0;
}

bool string_view_eq_cstr(StringView view, char const *cstr) {
    assert(cstr != NULL);
    size_t cstr_length = strlen(cstr);
    if (view.length != cstr_length) { return false; }
    return strncmp(view.data, cstr, view.length) == 0;
}

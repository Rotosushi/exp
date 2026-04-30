// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXP_SUPPORT_STRING_VIEW_H
#define EXP_SUPPORT_STRING_VIEW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Represents a non-owning view of a string
 *
 * @var data Pointer to the start of the string view
 * @var length The length of the string view
 */
typedef struct StringView {
    char const *data;
    size_t      length;
} StringView;

bool string_view_eq(StringView a, StringView b);
bool string_view_eq_cstr(StringView view, char const *cstr);

#define SV(str) ((StringView){.data = (str), .length = sizeof(str) - 1})

#endif // !EXP_SUPPORT_STRING_VIEW_H

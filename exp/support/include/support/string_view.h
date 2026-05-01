// SPDX-License-Identifier: MIT

/*!
 * \file string_view.h
 * \brief Defines the StringView type and member functions.
 */

#ifndef EXP_SUPPORT_STRING_VIEW_H
#define EXP_SUPPORT_STRING_VIEW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*! \struct StringView
 * \brief Represents a non-owning view of a string.
 */
typedef struct StringView {
    char const *data;
    size_t      length;
} StringView;

StringView  string_view();
StringView  string_view_create(char const *data, size_t length);
StringView  string_view_from_cstr(char const *cstr);
size_t      string_view_length(StringView view);
char const *string_view_cstr(StringView view);
int32_t     string_view_cmp(StringView a, StringView b);
int32_t     string_view_cmp_cstr(StringView view, char const *cstr);
bool        string_view_eq(StringView a, StringView b);
bool        string_view_eq_cstr(StringView view, char const *cstr);

/*! \def SV(str)
 * \brief Macro to create a StringView from a string literal.
 * The length is automatically calculated at compile time.
 */
#define SV(str) ((StringView){.data = (str), .length = sizeof(str) - 1})

#endif // !EXP_SUPPORT_STRING_VIEW_H

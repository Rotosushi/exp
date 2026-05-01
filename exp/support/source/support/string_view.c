// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file string_view.c
 * \brief Implements the StringView type and member functions.
 */

#include <assert.h>
#include <string.h>

#include "support/string_view.h"

/*!
 * \brief Creates an empty StringView.
 * \memberof StringView
 * \return An empty StringView with data set to NULL and length set to 0
 */
StringView string_view() { return (StringView){.data = NULL, .length = 0}; }

/*!
 * \brief Creates a StringView from a pointer and length.
 * \memberof StringView
 * \param data Pointer to the start of the string view
 * \param length The length of the string view
 * \return A StringView representing the specified data and length
 */
StringView string_view_create(char const *data, size_t length) {
    return (StringView){.data = data, .length = length};
}

/*!
 * \brief Creates a StringView from a null-terminated C string.
 * \memberof StringView
 * \param cstr A null-terminated C string
 * \return A StringView representing the specified C string
 */
StringView string_view_from_cstr(char const *cstr) {
    assert(cstr != NULL);
    return (StringView){.data = cstr, .length = strlen(cstr)};
}

/*!
 * \brief Returns the length of a StringView.
 * \memberof StringView
 * \param view The StringView
 * \return The length of the StringView
 */
size_t string_view_length(StringView view) { return view.length; }

/*!
 * \brief Converts a StringView to a null-terminated C string.
 * \memberof StringView
 * \warning The returned pointer is not null-terminated if the underlying
 *          cstring is not null-terminated.
 * \param view The StringView to convert
 * \return A pointer to a null-terminated C string representing the StringView
 *
 */
char const *string_view_cstr(StringView view) {
    assert(view.data != NULL);
    return view.data;
}

/*!
 * \brief Compares two StringViews lexicographically.
 * \memberof StringView
 *
 * \param a The first StringView
 * \param b The second StringView
 * \return A negative value if a < b, 0 if a == b, and a positive value if a > b
 */
int32_t string_view_cmp(StringView a, StringView b) {
    size_t min_length = a.length < b.length ? a.length : b.length;
    return strncmp(a.data, b.data, min_length);
}

/*!
 * \brief Compares a StringView to a null-terminated C string lexicographically.
 * \memberof StringView
 * \param view The StringView
 * \param cstr The null-terminated C string
 * \return A negative value if view < cstr, 0 if view == cstr, and a positive
 *         value if view > cstr
 */
int32_t string_view_cmp_cstr(StringView view, char const *cstr) {
    assert(cstr != NULL);
    size_t cstr_length = strlen(cstr);
    size_t min_length  = view.length < cstr_length ? view.length : cstr_length;
    return strncmp(view.data, cstr, min_length);
}

/*!
 * \brief Compares two StringViews for equality.
 * \memberof StringView
 * \param a The first StringView
 * \param b The second StringView
 * \return true if the StringViews are equal, false otherwise
 */
bool string_view_eq(StringView a, StringView b) {
    if (a.length != b.length) { return false; }
    return strncmp(a.data, b.data, a.length) == 0;
}

/*!
 * \brief Compares a StringView to a null-terminated C string for equality.
 * \memberof StringView
 * \param view The StringView
 * \param cstr The null-terminated C string
 * \return true if the StringView is equal to the C string, false otherwise
 */
bool string_view_eq_cstr(StringView view, char const *cstr) {
    assert(cstr != NULL);
    size_t cstr_length = strlen(cstr);
    if (view.length != cstr_length) { return false; }
    return strncmp(view.data, cstr, view.length) == 0;
}

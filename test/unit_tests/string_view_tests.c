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

#include "utility/result.h"
#include "utility/string_view.h"

i32 string_view_tests([[maybe_unused]] i32 argc,
                      [[maybe_unused]] char *argv[]) {
    ExpResult result = EXP_SUCCESS;

    StringView sv_0 = string_view_create();
    if (!string_view_empty(sv_0)) { result = EXP_FAILURE; }

    StringView sv_1 = string_view_from_str("", 0);
    if (!string_view_empty(sv_1)) { result = EXP_FAILURE; }
    if (!string_view_equality(sv_0, sv_1)) { result = EXP_FAILURE; }

    StringView sv_2 = string_view_from_cstring("");
    if (!string_view_empty(sv_2)) { result = EXP_FAILURE; }
    if (!string_view_equality(sv_2, sv_1)) { result = EXP_FAILURE; }
    if (!string_view_equality(sv_0, sv_2)) { result = EXP_FAILURE; }

    StringView sv_3 = SV("");
    if (!string_view_empty(sv_3)) { result = EXP_FAILURE; }
    if (!string_view_equality(sv_3, sv_2)) { result = EXP_FAILURE; }
    if (!string_view_equality(sv_3, sv_1)) { result = EXP_FAILURE; }
    if (!string_view_equality(sv_3, sv_0)) { result = EXP_FAILURE; }

    StringView sv_4 = string_view_from_str("Hello", 5);
    if (string_view_empty(sv_4)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_4, sv_3)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_4, sv_2)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_4, sv_1)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_4, sv_0)) { result = EXP_FAILURE; }

    StringView sv_5 = string_view_from_cstring("Hello");
    if (string_view_empty(sv_5)) { result = EXP_FAILURE; }
    if (!string_view_equality(sv_5, sv_4)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_5, sv_3)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_5, sv_2)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_5, sv_1)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_5, sv_0)) { result = EXP_FAILURE; }

    StringView sv_6 = SV("Hello");
    if (string_view_empty(sv_6)) { result = EXP_FAILURE; }
    if (!string_view_equality(sv_6, sv_5)) { result = EXP_FAILURE; }
    if (!string_view_equality(sv_6, sv_4)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_6, sv_3)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_6, sv_2)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_6, sv_1)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_6, sv_0)) { result = EXP_FAILURE; }

    StringView sv_7 = string_view_from_str("World", 5);
    if (string_view_empty(sv_7)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_7, sv_6)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_7, sv_5)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_7, sv_4)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_7, sv_3)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_7, sv_2)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_7, sv_1)) { result = EXP_FAILURE; }
    if (string_view_equality(sv_7, sv_0)) { result = EXP_FAILURE; }

    return result;
}

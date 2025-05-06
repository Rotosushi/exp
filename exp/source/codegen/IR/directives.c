/**
 * Copyright (C) 2025 cade
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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "codegen/IR/directives.h"

void ir_directive_version(StringView version, String *restrict string) {
    string_append(string, SV(".version "));
    string_append(string, version);
    string_append(string, SV("\n"));
}

void ir_directive_file(StringView path, String *restrict string) {
    string_append(string, SV(".file "));
    string_append(string, path);
    string_append(string, SV("\n"));
}

void ir_directive_let(StringView name, String *restrict string) {
    string_append(string, SV(".let"));
    string_append(string, name);
}

void ir_directive_comment(StringView comment, String *restrict string) {
    string_append(string, SV("// "));
    string_append(string, comment);
    string_append(string, SV("\n"));
}

/**
 * Copyright (C) 2025 Cade Weinberg
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

/**
 * @file targets/x86_64/print/mnemonic.c
 */

#include "targets/x86_64/print/mnemonic.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

static bool valid_operand_size(u8 size) {
    return (size == 1) || (size == 2) || (size == 4) || (size == 8);
}

void print_x86_64_mnemonic(String *buffer, StringView mnemonic, u8 size) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(valid_operand_size(size));
    // #NOTE: x86 instructions can be postfixed with a size mnemonic
    //  to allow the assembler to emit more explicit instructions.
    //  the mnemonic makes explicit the size of the operands involved.
    //  technically it is only required when the size cannot be inferred
    //  by the operands themselves (such as on operand being a 64 bit register)
    //  however it is good practice to be as explicit as possible.
    //  'b' -> 1 byte
    //  'w' -> 2 byte
    //  'l' -> 4 byte
    //  'q' -> 8 byte
    //  #NOTE: floating point types are not supported yet, but they have their
    //   own size mnemonics
    //   's' -> f32
    //   'l' -> f64
    //   't' -> f80
    string_append(buffer, SV("  "));
    string_append(buffer, mnemonic);
    switch (size) {
    case 1:  string_append(buffer, SV("b")); break;
    case 2:  string_append(buffer, SV("w")); break;
    case 4:  string_append(buffer, SV("l")); break;
    case 8:  string_append(buffer, SV("q")); break;
    default: EXP_UNREACHABLE();
    }
    string_append(buffer, SV("\t"));
}

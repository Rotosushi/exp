// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_IMR_BYTECODE_H
#define EXP_IMR_BYTECODE_H
#include <stddef.h>
#include <stdint.h>

/**
 * @brief represents a section of instructions.
 *
 */
typedef struct Bytecode {
  size_t length;
  size_t capacity;
  uint8_t *buffer;
} Bytecode;

/**
 * @brief create a new Bytecode
 *
 * @return Bytecode
 */
Bytecode bytecode_create();
/**
 * @brief destroy a Bytecode
 *
 * @param bytecode
 */
void bytecode_destroy(Bytecode *restrict bytecode);

/**
 * @brief emit a stop instruction
 *
 * @param bytecode
 */
void bytecode_emit_stop(Bytecode *restrict bytecode);

/**
 * @brief push a constant onto the stack
 *
 * @param bytecode the bytecode to emit into
 * @param name_index the index of the constant to push
 */
void bytecode_emit_push_constant(Bytecode *restrict bytecode,
                                 size_t name_index);

// void bytecode_emit_push_register(Bytecode *restrict bytecode,
//                                  uint8_t register_index);

void bytecode_emit_pop(Bytecode *restrict bytecode);

// void bytecode_emit_pop_register(Bytecode *restrict bytecode,
//                                 uint8_t register_index);

// void bytecode_emit_move_constant_to_register(Bytecode *restrict bytecode,
//                                              uint8_t register_index,
//                                              size_t constant_index);

/**
 * @brief define a global constant
 *
 * @param bytecode
 * @param name_index
 * @param type_index
 * @param value_index
 */
void bytecode_emit_define_global_constant(Bytecode *restrict bytecode);

#endif // !EXP_IMR_BYTECODE_H
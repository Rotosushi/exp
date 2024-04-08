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

void bytecode_clone(Bytecode *target, Bytecode *source);

bool bytecode_equality(Bytecode *b1, Bytecode *b2);

/**
 * @brief read the immediate value at the given index within
 * the given bytecode
 *
 * @warning does not understand where instructions start/end
 * all this function does is read the next few bytes starting
 * from the given offset.
 *
 * @param bytecode
 * @param offset
 * @param bytes
 * @return size_t
 */
size_t bytecode_read_immediate(Bytecode *restrict bytecode, size_t offset,
                               size_t bytes);

/**
 * @brief emit a stop instruction
 *
 * @param bytecode
 */
void bytecode_emit_stop(Bytecode *restrict bytecode);

/**
 * @brief emit a return instruction
 *
 * @param bytecode
 */
void bytecode_emit_return(Bytecode *restrict bytecode);

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

/**
 * @brief emit a unop negate '-'
 *
 * @param bytecode
 */
void bytecode_emit_unop_minus(Bytecode *restrict bytecode);

/**
 * @brief emit a binop '+'
 *
 * @param bytecode
 */
void bytecode_emit_binop_plus(Bytecode *restrict bytecode);

/**
 * @brief emit a binop '-'
 *
 * @param bytecode
 */
void bytecode_emit_binop_minus(Bytecode *restrict bytecode);

/**
 * @brief emit a binop '*'
 *
 * @param bytecode
 */
void bytecode_emit_binop_star(Bytecode *restrict bytecode);

/**
 * @brief emit a binop '/'
 *
 * @param bytecode
 */
void bytecode_emit_binop_slash(Bytecode *restrict bytecode);

/**
 * @brief emit a binop '%'
 *
 * @param bytecode
 */
void bytecode_emit_binop_percent(Bytecode *restrict bytecode);

#endif // !EXP_IMR_BYTECODE_H
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
#include "utility/int_types.h"

/**
 * @brief represents a section of instructions.
 *
 */
typedef struct Bytecode {
  u64 length;
  u64 capacity;
  u8 *buffer;
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
 * @return u64
 */
u64 bytecode_read_immediate(Bytecode *restrict bytecode, u64 offset, u64 bytes);

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
void bytecode_emit_push_constant(Bytecode *restrict bytecode, u64 name_index);

// void bytecode_emit_push_register(Bytecode *restrict bytecode,
//                                  u8 register_index);

void bytecode_emit_pop(Bytecode *restrict bytecode);

// void bytecode_emit_pop_register(Bytecode *restrict bytecode,
//                                 u8 register_index);

// void bytecode_emit_move_constant_to_register(Bytecode *restrict bytecode,
//                                              u8 register_index,
//                                              u64 constant_index);

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
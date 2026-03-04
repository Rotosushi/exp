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
  * along with exp.  If not, see <https://www.gnu.org/licenses/>.                
  */
#ifndef EXP_IMR_VALUE_FUNCTION_BODY_H
#define EXP_IMR_VALUE_FUNCTION_BODY_H

#include "codegen/x86/imr/value/function/block.h"

typedef struct Body {
    u32 size;
    u32 capacity;
    Block *buffer;
} Body;

void body_create(Body *restrict body);
void body_destroy(Body *restrict body);

// The body of a function is constructed of a set of 
// blocks. each block will end with an instruction which
// jumps to another target, within the body, to another 
// function, or returning from the current function.
//
// We need to support creating and insterting blocks within 
// the body of the function.
// as well as including a print subroutine for inspection.

block *body_insert(Body *restrict body, u32 index);
block *body_prepend(Body *restrict body);
block *body_append(Body *restrict body);

block *body_at(Body *restrict body, u32 index);

struct Context;
struct Function;
void print_body(String *restrict string,
                Body const *restrict body,
                struct Function *restrict function,
                struct Context *restrict context);

#endif // !EXP_IMR_VALUE_FUNCTION_BODY_H


// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_INTRINSICS_TYPE_OF_H
#define EXP_INTRINSICS_TYPE_OF_H

#include "env/context.h"
#include "imr/function.h"
#include "imr/type.h"
#include "imr/value.h"

Type const *type_of_operand(OperandKind kind, OperandData data,
                            Function *function, Context *context);
Type const *type_of_value(Value *value, Function *function, Context *context);
Type const *type_of_function(Function *body, Context *context);

#endif // !EXP_INTRINSICS_TYPE_OF_H

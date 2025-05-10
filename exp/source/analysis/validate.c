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

#include "analysis/validate.h"
#include "env/context.h"
#include "imr/value.h"
#include "support/assert.h"
#include "support/constant_string.h"
#include "support/unreachable.h"

static bool validate_local(Local const *restrict local,
                           u32 block_index,
                           Function const *restrict function) {
    exp_assert_always(local->type == NULL);
    Lifetime bounds = {.start = 0, .end = function->body.length};
    exp_assert_always(local->lifetime.start > local->lifetime.end);
    exp_assert_always(local->lifetime.start < bounds.start);
    exp_assert_always(local->lifetime.end >= bounds.end);
    exp_assert_always(local->lifetime.start <= block_index);
    return true;
}

static bool validate_constant(Value const *restrict constant,
                              u32 block_index,
                              Function const *restrict function,
                              Context *restrict context);

static bool validate_global(Symbol *restrict global,
                            u32 block_index,
                            Function const *restrict function,
                            Context *restrict context) {
    exp_assert_always(!string_view_empty(global->name));
    exp_assert_always(global->type != NULL);
    return validate_constant(global->value, block_index, function, context);
}

static bool validate_operand(OperandKind kind,
                             OperandData data,
                             u32         block_index,
                             Function const *restrict function,
                             Context *restrict context);

static bool validate_tuple(Tuple const *restrict tuple,
                           u32 block_index,
                           Function const *restrict function,
                           Context *restrict context) {
    for (u32 index = 0; index < tuple->size; ++index) {
        Operand element = tuple->elements[index];
        if (!validate_operand(
                element.kind, element.data, block_index, function, context)) {
            return false;
        }
    }
    return true;
}

static bool validate_constant(Value const *restrict constant,
                              u32 block_index,
                              Function const *restrict function,
                              Context *restrict context) {
    switch (constant->kind) {
    case VALUE_KIND_TUPLE:
        return validate_tuple(&constant->tuple, block_index, function, context);

    case VALUE_KIND_FUNCTION: return validate(&constant->function, context);

    // There is nothing to validate about a scalar value in and of itself.
    default: return true;
    }
}

static bool validate_label(ConstantString const *restrict label,
                           u32 block_index,
                           Function const *restrict function,
                           Context *restrict context) {
    StringView name = constant_string_to_view(label);

    LookupResult lookup = context_lookup_label(context, function, name);

    switch (lookup.kind) {
    // #TODO: handle name usage before definition.
    case LOOKUP_RESULT_NONE:
        PANIC("validate label, label is not associated with any definition");

    case LOOKUP_RESULT_LOCAL:
        return validate_local(lookup.local, block_index, function);

    case LOOKUP_RESULT_GLOBAL:
        return validate_global(lookup.global, block_index, function, context);

    default: EXP_UNREACHABLE();
    }

    return true;
}

static bool validate_operand(OperandKind kind,
                             OperandData data,
                             u32         block_index,
                             Function const *restrict function,
                             Context *restrict context) {
    switch (kind) {
    case OPERAND_KIND_SSA:
        return validate_local(
            function_lookup_local(function, data.ssa), block_index, function);

    case OPERAND_KIND_CONSTANT:
        return validate_constant(data.constant, block_index, function, context);

    case OPERAND_KIND_LABEL:
        return validate_label(data.label, block_index, function, context);

    // #NOTE: There is nothing to validate about an immediate scalar value
    // in and of itself.
    default: return true;
    }
}

static bool validate_operand_A(Instruction instruction,
                               u32         block_index,
                               Function const *restrict function) {
    switch (instruction.A_kind) {
    case OPERAND_KIND_SSA: {
        Local *local = function_lookup_local(function, instruction.A_data.ssa);
        validate_local(local, block_index, function);
        // #NOTE: in addition to catching the mismatch between the declaration
        //  of a particular local and the lifetime of that local, this check
        //  also ensures that each ssa only appears in position A once in the
        //  block. for the local to appear a second time implies appearing after
        //  the first time, thus block_index would have to be greater than
        //  first_use. if it's less than, this is also an error, which is why we
        //  use == over <
        exp_assert_always(local->lifetime.start == block_index);
        exp_assert_always(local->lifetime.end >= block_index);
        break;
    }

    default: {
        // #NOTE: Scalars (i8, u16, etc...) in position A are not currently
        // allowed, Though such a thing could be used by jump instructions.
        // #NOTE: Constants (Value const *) in position A are not currently
        // allowed, Though a Tuple in position A could encode multiple
        // declaration statements. `let (a, b, c) = f(x)`
        // #NOTE: Labels in position A are not currently allowed, Though
        // such a thing could allow use to compile some expressions down
        // to fewer instructions. `let a = b + c;` could become `add %a, %b, %c`
        // instead of `add %0, %b, %c; let %a, %0`. Or could encode a
        // write to a global label.
        PANIC("Validate: Operand A, The destination operand, does not "
              "represent a local.");
    }
    }
    return true;
}

bool validate_B(Instruction instruction,
                u32         block_index,
                Function const *restrict function,
                Context *restrict context) {
    if (!validate_operand(instruction.B_kind,
                          instruction.B_data,
                          block_index,
                          function,
                          context)) {
        return false;
    }

    return true;
}

bool validate_AB(Instruction instruction,
                 u32         block_index,
                 Function const *restrict function,
                 Context *restrict context) {
    if (!validate_operand_A(instruction, block_index, function)) {
        return false;
    }

    if (!validate_operand(instruction.B_kind,
                          instruction.B_data,
                          block_index,
                          function,
                          context)) {
        return false;
    }

    return true;
}

bool validate_ABC(Instruction instruction,
                  u32         block_index,
                  Function const *restrict function,
                  Context *restrict context) {
    if (!validate_AB(instruction, block_index, function, context)) {
        return false;
    }

    if (!validate_operand(instruction.C_kind,
                          instruction.C_data,
                          block_index,
                          function,
                          context)) {
        return false;
    }

    return true;
}

bool validate_ret(Instruction instruction,
                  u32         block_index,
                  Function const *restrict function,
                  Context *restrict context) {
    exp_assert_always(instruction.opcode == OPCODE_RET);

    if (!validate_B(instruction, block_index, function, context)) {
        return false;
    }

    Type const *B_type = context_type_of_operand(
        context, function, operand(instruction.B_kind, instruction.B_data));
    exp_assert_always(B_type != NULL);

    exp_assert_always(type_equality(B_type, function->return_type));

    return true;
}

bool validate_call(Instruction instruction,
                   u32         block_index,
                   Function const *restrict function,
                   Context *restrict context) {
    exp_assert_always(instruction.opcode == OPCODE_CALL);

    if (!validate_ABC(instruction, block_index, function, context)) {
        return false;
    }
}

bool validate(Function const *restrict expression, Context *restrict context) {}

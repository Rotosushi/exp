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
#include "imr/function.h"
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
    for (u32 index = 0; index < tuple->length; ++index) {
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

static bool validate_B(Instruction instruction,
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

static bool validate_AB(Instruction instruction,
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

static bool validate_ABC(Instruction instruction,
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

static bool validate_ret(Instruction instruction,
                         u32         block_index,
                         Function const *restrict function,
                         Context *restrict context) {
    exp_assert_always(instruction.opcode == OPCODE_RET);

    if (!validate_B(instruction, block_index, function, context)) {
        return false;
    }

    Type const *B_type =
        context_type_of_operand(context, function, operand_B(instruction));
    exp_assert_always(B_type != NULL);

    exp_assert_always(type_equality(B_type, function->return_type));

    return true;
}

static bool validate_call(Instruction instruction,
                          u32         block_index,
                          Function const *restrict function,
                          Context *restrict context) {
    exp_assert_always(instruction.opcode == OPCODE_CALL);

    if (!validate_ABC(instruction, block_index, function, context)) {
        return false;
    }

    Type const *A_type =
        context_type_of_operand(context, function, operand_A(instruction));

    Type const *B_type =
        context_type_of_operand(context, function, operand_B(instruction));
    exp_assert_always(B_type != NULL);
    exp_assert_always(B_type->kind == TYPE_KIND_FUNCTION);
    FunctionType const *callee = &B_type->function;

    Type const *return_type = callee->return_type;
    exp_assert_always(return_type != NULL);
    exp_assert_always(type_equality(return_type, A_type));

    TupleType const *formal_args = &callee->argument_types;

    Type const *C_type =
        context_type_of_operand(context, function, operand_C(instruction));
    exp_assert_always(C_type != NULL);
    exp_assert_always(C_type->kind == TYPE_KIND_TUPLE);
    TupleType const *actual_args = &C_type->tuple;

    exp_assert_always(formal_args->length == actual_args->length);

    for (u32 index = 0; index < formal_args->length; ++index) {
        Type const *formal_arg = formal_args->types[index];
        exp_assert_always(formal_arg != NULL);
        Type const *actual_arg = actual_args->types[index];
        exp_assert_always(actual_arg != NULL);

        exp_assert_always(type_equality(formal_arg, actual_arg));
    }

    return true;
}

static bool validate_let(Instruction instruction,
                         u32         block_index,
                         Function const *restrict function,
                         Context *restrict context) {
    if (!validate_AB(instruction, block_index, function, context)) {
        return false;
    }

    Type const *A_type =
        context_type_of_operand(context, function, operand_A(instruction));
    exp_assert_always(A_type != NULL);

    Type const *B_type =
        context_type_of_operand(context, function, operand_B(instruction));
    exp_assert_always(B_type != NULL);

    exp_assert_always(type_equality(A_type, B_type));
    return true;
}

static bool validate_unop(Instruction instruction,
                          u32         block_index,
                          Function const *restrict function,
                          Context *restrict context,
                          Type const *return_type,
                          Type const *arg_type) {
    if (!validate_AB(instruction, block_index, function, context)) {
        return false;
    }

    Type const *A_type =
        context_type_of_operand(context, function, operand_A(instruction));
    exp_assert_always(type_equality(A_type, return_type));

    Type const *B_type =
        context_type_of_operand(context, function, operand_B(instruction));
    exp_assert_always(B_type != NULL);
    exp_assert_always(type_equality(B_type, arg_type));
    return true;
}

static bool validate_arithmetic_unop(Instruction instruction,
                                     u32         block_index,
                                     Function const *restrict function,
                                     Context *restrict context) {
    // #NOTE: We only allow arithmetic operations [+ - * / %] on
    // itegral types. The question is, which integral type are we concerned
    // about here? Currently, we only allow arithmetic if the types match
    // exactly. and I think the answer going forward is to promote integral
    // types to something larger if that is needed, for example to compile [u8 +
    // u32]. I think our solution here is to rely on the type of the incoming
    // argument. If it matches a valid type for an arithmetic expression, then
    // we require the return type to match exactly. Otherwise this is an invalid
    // instruction.
    Type const *underlying_type =
        context_type_of_operand(context, function, operand_B(instruction));
    exp_assert_always(underlying_type != NULL);
    switch (underlying_type->kind) {
    case TYPE_KIND_U8:
    case TYPE_KIND_U16:
    case TYPE_KIND_U32:
    case TYPE_KIND_U64:
    case TYPE_KIND_I8:
    case TYPE_KIND_I16:
    case TYPE_KIND_I32:
    case TYPE_KIND_I64:
        return validate_unop(instruction,
                             block_index,
                             function,
                             context,
                             underlying_type,
                             underlying_type);

    default: PANIC("Underlying type of unop is not supported");
    }
}

static bool validate_dot(Instruction instruction,
                         u32         block_index,
                         Function const *restrict function,
                         Context *restrict context) {
    if (!validate_ABC(instruction, block_index, function, context)) {
        return false;
    }

    Type const *A_type =
        context_type_of_operand(context, function, operand_A(instruction));

    Type const *B_type =
        context_type_of_operand(context, function, operand_B(instruction));
    exp_assert_always(type_is_indexable(B_type));

    exp_assert_always(B_type->kind == TYPE_KIND_TUPLE);
    TupleType const *tuple = &B_type->tuple;

    Type const *C_type =
        context_type_of_operand(context, function, operand_C(instruction));
    exp_assert_always(type_is_index(C_type));

    exp_assert_always(operand_is_index(operand_C(instruction)));
    u64 index = operand_as_index(operand_C(instruction));
    exp_assert_always(index <= u32_MAX);
    exp_assert_always(tuple_type_index_in_bounds(tuple, (u32)index));

    Type const *element_type = tuple_type_at(tuple, (u32)index);
    exp_assert_always(type_equality(element_type, A_type));
    return true;
}

static bool validate_binop(Instruction instruction,
                           u32         block_index,
                           Function const *restrict function,
                           Context *restrict context,
                           Type const *return_type,
                           Type const *left_type,
                           Type const *right_type) {
    if (!validate_ABC(instruction, block_index, function, context)) {
        return false;
    }

    Type const *A_type =
        context_type_of_operand(context, function, operand_A(instruction));
    exp_assert_always(type_equality(A_type, return_type));

    Type const *B_type =
        context_type_of_operand(context, function, operand_B(instruction));
    exp_assert_always(B_type != NULL);
    exp_assert_always(type_equality(B_type, left_type));

    Type const *C_type =
        context_type_of_operand(context, function, operand_C(instruction));
    exp_assert_always(C_type != NULL);
    exp_assert_always(type_equality(C_type, right_type));

    return true;
}

static bool validate_arithmetic_binop(Instruction instruction,
                                      u32         block_index,
                                      Function const *restrict function,
                                      Context *restrict context) {
    Type const *underlying_type =
        context_type_of_operand(context, function, operand_B(instruction));
    exp_assert_always(underlying_type != NULL);
    switch (instruction.B_kind) {
    case OPERAND_KIND_U8:
    case OPERAND_KIND_U16:
    case OPERAND_KIND_U32:
    case OPERAND_KIND_U64:
    case OPERAND_KIND_I8:
    case OPERAND_KIND_I16:
    case OPERAND_KIND_I32:
    case OPERAND_KIND_I64:
        return validate_binop(instruction,
                              block_index,
                              function,
                              context,
                              underlying_type,
                              underlying_type,
                              underlying_type);

    default: PANIC("Underlying type of binop not supported");
    }
}

static bool validate_instruction(Instruction instruction,
                                 u32         block_index,
                                 Function const *restrict function,
                                 Context *restrict context) {
    switch (instruction.opcode) {
    case OPCODE_RET:
        return validate_ret(instruction, block_index, function, context);
    case OPCODE_CALL:
        return validate_call(instruction, block_index, function, context);
    case OPCODE_LET:
        return validate_let(instruction, block_index, function, context);
    case OPCODE_NEG:
        return validate_arithmetic_unop(
            instruction, block_index, function, context);
    case OPCODE_DOT:
        return validate_dot(instruction, block_index, function, context);
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_DIV:
    case OPCODE_MOD:
        return validate_arithmetic_binop(
            instruction, block_index, function, context);

    default: EXP_UNREACHABLE();
    }
}

bool validate(Function const *restrict expression, Context *restrict context) {
    exp_assert(expression != NULL);
    exp_assert(context != NULL);

    Bytecode const *block = &expression->body;
    for (u32 index = 0; index < block->length; ++index) {
        if (!validate_instruction(
                block->buffer[index], index, expression, context)) {
            return false;
        }
    }

    return true;
}

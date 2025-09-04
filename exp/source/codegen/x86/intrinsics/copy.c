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
#include <assert.h>

#include "codegen/x86/imr/function.h"
#include "codegen/x86/intrinsics/copy.h"
#include "imr/type.h"
#include "support/assert.h"
#include "support/message.h"
#include "support/unreachable.h"

static void x86_codegen_copy_operand(x86_Location dst,
                                     Operand      operand,
                                     u64          block_index,
                                     x86_Function *restrict function,
                                     Context *restrict context);

static void x86_codegen_copy_tuple(x86_Location dst,
                                   Value const *restrict value,
                                   u64 block_index,
                                   x86_Function *restrict function,
                                   Context *restrict context);

// if the size to copy is larger than some limit, we want to
// call the builtin memcpy library function. This function
// is always linked into the resulting binary.
void x86_codegen_copy(x86_Location dst,
                      x86_Location src,
                      Type const  *type,
                      u64          block_index,
                      x86_Function *restrict x86_function,
                      Context *restrict context) {
    if (!dst.is_address || !src.is_address) {
        // if one or the other allocations is not an address, then they must be
        // register sized, because they are allocated to a register, and since
        // one of them is in a register, or both of them are we are able to
        // construct a valid x86 mov instruction to handle the copy.
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_location(src)));
        return;
    }

    // otherwise we have to handle the copy, mem to mem.
    // we can call out to the memcpy library routine.
    // or we can emit instructions to manually copy the
    // data. it's our choice here.
}

void x86_codegen_copy_value(x86_Location dst,
                            Value const *restrict value,
                            u64 block_index,
                            x86_Function *restrict x86_function,
                            Context *restrict context) {
    // if the value is a primary type then we can use an instruction
    // to initialize the local in whatever location it resides.
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: break; // don't initialize
    case VALUE_KIND_NIL:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_nil()));
        break;

    case VALUE_KIND_BOOL:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_bool(value->bool_)));
        break;

    case VALUE_KIND_U8:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_u8(value->u8_)));
        break;

    case VALUE_KIND_U16:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_u16(value->u16_)));
        break;

    case VALUE_KIND_U32:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_u32(value->u32_)));
        break;

    case VALUE_KIND_U64:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_u64(value->u64_)));
        break;

    case VALUE_KIND_I8:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_i8(value->i8_)));
        break;

    case VALUE_KIND_I16:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_i16(value->i16_)));
        break;

    case VALUE_KIND_I32:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_i32(value->i32_)));
        break;

    case VALUE_KIND_I64:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_i64(value->i64_)));
        break;

    case VALUE_KIND_TUPLE:
        x86_codegen_copy_tuple(dst, value, block_index, x86_function, context);
        break;

    case VALUE_KIND_FUNCTION:
        // #NOTE: We allow local functions via lambdas. Lambdas without support
        // for capturing any context are equivalent to function pointers. so we
        // silently implement them as such here.
        exp_assert(value->kind == VALUE_KIND_FUNCTION);
        Function const *function = &value->function;
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_rip(function->name)));
        break;

    default: EXP_UNREACHABLE();
    }
}

static void x86_codegen_copy_operand(x86_Location dst,
                                     Operand      operand,
                                     u64          block_index,
                                     x86_Function *restrict x86_function,
                                     Context *restrict context) {
    switch (operand.kind) {
    case OPERAND_KIND_LABEL: {
        x86_Allocation *local =
            x86_function_allocation_named(x86_function, operand.data.label);
        exp_assert(local != NULL);
        // #TODO: global lookup can become copy from
        // rip relative address. whats tricky is the type to copy.
        x86_codegen_copy(dst,
                         local->location,
                         local->type,
                         block_index,
                         x86_function,
                         context);
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        x86_codegen_copy_value(
            dst, operand.data.constant, block_index, x86_function, context);
        break;
    }

    case OPERAND_KIND_SSA: {
        x86_Allocation *local =
            x86_function_allocation_at(x86_function, operand.data.ssa);
        exp_assert(local != NULL);
        x86_codegen_copy(dst,
                         local->location,
                         local->type,
                         block_index,
                         x86_function,
                         context);
        break;
    }

    case OPERAND_KIND_NIL:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(dst), x86_operand_nil()));
        break;

    case OPERAND_KIND_BOOL:
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_bool(operand.data.bool_)));
        break;

    case OPERAND_KIND_U8:
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_u8(operand.data.u8_)));
        break;

    case OPERAND_KIND_U16:
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_u16(operand.data.u16_)));
        break;

    case OPERAND_KIND_U32:
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_u32(operand.data.u32_)));
        break;

    case OPERAND_KIND_U64:
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_u64(operand.data.u64_)));
        break;

    case OPERAND_KIND_I8:
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_i8(operand.data.i8_)));
        break;

    case OPERAND_KIND_I16:
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_i16(operand.data.i16_)));
        break;

    case OPERAND_KIND_I32:
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_i32(operand.data.i32_)));
        break;

    case OPERAND_KIND_I64:
        x86_function_append(x86_function,
                            x86_mov(x86_operand_location(dst),
                                    x86_operand_i64(operand.data.i64_)));
        break;

    default: unreachable();
    }
}

static void x86_codegen_copy_tuple(x86_Location dst,
                                   Value const *restrict value,
                                   u64 block_index,
                                   x86_Function *restrict function,
                                   Context *restrict context) {
    exp_assert(value->kind == VALUE_KIND_TUPLE);
    Tuple const *tuple = &value->tuple;
    Type const  *type  = value->type;
    exp_assert(type->kind == TYPE_KIND_COMPOSITE);
    exp_assert(type->composite.kind == TYPE_COMPOSITE_KIND_TUPLE);
    Layout const *layout = type->layout;
    exp_assert(layout->kind == LAYOUT_KIND_TUPLE);
    LayoutTuple const *layout_tuple = &layout->data.tuple;

    for (u32 index = 0; index < tuple->length; ++index) {
        u64 element_offset =
            layout_tuple_get_element_offset(layout_tuple, index);
        exp_assert_always(element_offset < i32_MAX);
        x86_Location element_address =
            x86_location_address_increment(dst, (i32)element_offset);
        x86_codegen_copy_operand(element_address,
                                 tuple->elements[index],
                                 block_index,
                                 function,
                                 context);
    }
}

// void x86_codegen_copy_scalar_memory(x86_Address *restrict dst,
//                                     x86_Address *restrict src,
//                                     u64 size,
//                                     u64 Idx,
//                                     x86_Context *restrict context) {
//     exp_assert(x86_gpr_valid_size(size));
//     x86_GPR gpr = x86_context_aquire_any_gpr(context, size, Idx);

//     x86_context_append(
//         context, x86_mov(x86_operand_gpr(gpr), x86_operand_address(*src)));
//     x86_context_append(
//         context, x86_mov(x86_operand_address(*dst), x86_operand_gpr(gpr)));

//     x86_context_release_gpr(context, gpr, Idx);
// }

// void x86_codegen_copy_composite_memory(x86_Address *restrict dst,
//                                        x86_Address *restrict src,
//                                        Type const *type,
//                                        u64         Idx,
//                                        x86_Context *restrict context) {
//     assert(type->kind == TYPE_KIND_TUPLE);
//     TupleType const *tuple_type = &type->tuple;

//     x86_Address dst_element_address = *dst;
//     x86_Address src_element_address = *src;
//     for (u64 i = 0; i < tuple_type->length; ++i) {
//         Type const *element_type = tuple_type->types[i];
//         u64         element_size = size_of(element_type);

//         if (type_is_scalar(element_type)) {
//             x86_codegen_copy_scalar_memory(&dst_element_address,
//                                            &src_element_address,
//                                            element_size,
//                                            Idx,
//                                            context);
//         } else {
//             x86_codegen_copy_composite_memory(&dst_element_address,
//                                               &src_element_address,
//                                               element_type,
//                                               Idx,
//                                               context);
//         }

//         assert(element_size <= i64_MAX);
//         i64 offset = (i64)element_size;
//         dst_element_address.offset += offset;
//         src_element_address.offset += offset;
//     }
// }

// void x86_codegen_copy_memory(x86_Address *restrict dst,
//                              x86_Address *restrict src,
//                              Type const *type,
//                              u64         Idx,
//                              x86_Context *restrict context) {
//     if (type_is_scalar(type)) {
//         u64 size = size_of(type);
//         x86_codegen_copy_scalar_memory(dst, src, size, Idx, context);
//     } else {
//         x86_codegen_copy_composite_memory(dst, src, type, Idx, context);
//     }
// }

// void x86_codegen_copy_allocation_from_memory(x86_Allocation *restrict dst,
//                                              x86_Address *restrict src,
//                                              Type const *restrict type,
//                                              u64 Idx,
//                                              x86_Context *restrict context) {
//     if (dst->location.kind == X86_LOCATION_ADDRESS) {
//         x86_codegen_copy_memory(
//             &dst->location.address, src, type, Idx, context);
//     } else {
//         x86_context_append(context,
//                            x86_mov(x86_operand_gpr(dst->location.gpr),
//                                    x86_operand_address(*src)));
//     }
// }

// static void x86_codegen_copy_scalar_allocation(x86_Allocation *restrict dst,
//                                                x86_Allocation *restrict src,
//                                                u64 Idx,
//                                                x86_Context *restrict context)
//                                                {
//     if ((dst->location.kind == X86_LOCATION_GPR) ||
//         (src->location.kind == X86_LOCATION_GPR)) {
//         x86_context_append(
//             context, x86_mov(x86_operand_alloc(dst),
//             x86_operand_alloc(src)));
//     } else {
//         u64 size = size_of(dst->type);
//         x86_codegen_copy_scalar_memory(
//             &dst->location.address, &src->location.address, size, Idx,
//             context);
//     }
// }

// void x86_codegen_copy_allocation(x86_Allocation *restrict dst,
//                                  x86_Allocation *restrict src,
//                                  u64 Idx,
//                                  x86_Context *restrict context) {
//     assert(type_equality(dst->type, src->type));

//     if (x86_location_equality(dst->location, src->location)) { return; }

//     if (type_is_scalar(dst->type)) {
//         x86_codegen_copy_scalar_allocation(dst, src, Idx, context);
//     } else {
//         assert(dst->location.kind == X86_LOCATION_ADDRESS);
//         assert(src->location.kind == X86_LOCATION_ADDRESS);

//         x86_codegen_copy_composite_memory(&dst->location.address,
//                                           &src->location.address,
//                                           dst->type,
//                                           Idx,
//                                           context);
//     }
// }

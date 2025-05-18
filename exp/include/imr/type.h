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
#ifndef EXP_IMR_TYPE_H
#define EXP_IMR_TYPE_H
#include <stdbool.h>

#include "support/string.h"

typedef enum TypeKind {
    TYPE_KIND_NIL,
    TYPE_KIND_BOOL,
    TYPE_KIND_U8,
    TYPE_KIND_U16,
    TYPE_KIND_U32,
    TYPE_KIND_U64,
    TYPE_KIND_I8,
    TYPE_KIND_I16,
    TYPE_KIND_I32,
    TYPE_KIND_I64,
    TYPE_KIND_TUPLE,
    TYPE_KIND_FUNCTION,
} TypeKind;

struct Type;

typedef struct TupleType {
    u32                 length;
    u32                 capacity;
    struct Type const **types;
} TupleType;

void tuple_type_create(TupleType *restrict tuple);
void tuple_type_destroy(TupleType *restrict tuple_type);
bool tuple_type_equal(TupleType const *A, TupleType const *B);
bool tuple_type_index_in_bounds(TupleType const *restrict tuple, u32 index);
void tuple_type_append(TupleType *restrict tuple_type, struct Type const *type);
struct Type const *tuple_type_at(TupleType const *restrict tuple, u32 index);

typedef struct FunctionType {
    struct Type const *return_type;
    TupleType          argument_types;
} FunctionType;

bool function_type_equal(FunctionType const *A, FunctionType const *B);

/**
 * @brief represents Types in the compiler
 *
 * #TODO: #FEATURE: type attributes, something like u16::max, could work in the
 * same way that struct members work, there is also no reason we cannot bind a
 * function ptr there as well, giving type "member" functions. except not tied
 * to a specific instance of that type. I think we can leverage such a mechanism
 * for type introspection if we implicitly fill in the member details when we
 * create the type. That is, a hash table which is implicitly filled in with
 * the data about the type
 * What syntax do we want?
 * well, why not use the already existing dot syntax. something like
 * `i16.min` or would it be `i16.min()`
 * and would it be `tuple-type.members` or `tuple-type.members()`
 *
 * So, if each type gets a hash-table associated with it. Where we fill in
 * attributes about said type. What is the type of an element of that hash
 * table?
 * I mean, the only viable choice is a Value isn't it? That is the
 * existing structure which wraps up any representable comptime value right?
 * So how close is it to a symbol table? Isn't it just a symbol-table?
 * like, a struct type, which is equivalent to a tuple type other than the
 * method by which elements are accessed, can be written in terms of a tuple if
 * we simply associate an index with a name via a symbol-table. Does this
 * require a special type/value to communicate this fact to the interpreter
 * codegenerator? Well, maybe, it depends on wether or not the mere fact that we
 * are handling a structure type in and of itself is enough to disambiguate that
 * a u64 element within that structure is an index into the underlying tuple, or
 * is simply a size attribute or somthing else.
 * Because the index as I have described it is the name, which looks up a
 * symbol, which holds a reference to the value, which holds the index of the
 * associated element within the underlying tuple.
 * Just something to consider: structure types generally have less than 20
 * members. So would a simple associative array work just as well, a bit
 * faster and less memory overhead?
 * Alternatively, The structre type could have the member-element association
 * alongside a symbol-table which describes attributes? Or the member-element
 * association could itself be a single element within the symbol-table?
 *
 * We can easily store a reference to a user defined function within this
 * local symbol-table as well, given that we store a reference to a value,
 * and functions are stored as values.
 *
 * Which leads me to a question, what about compiler intrinsics?
 * for instance, it seems natural to store a "size" attribute within
 * this symbol-table, which can be queried as `type.size` however,
 * I think it's perfectly acceptable to also provide an intrinsic function
 * such as `size(type)` and allow the programmer to choose their preference.
 * If we can unify the underlying implementation, i.e. delegate to the
 * `context_size_of(context type)` function, or fill in the symbol with the
 * result of a call to the `context_size_of(context, type)` function, then we
 * can implement the comptime `size(type)` function in terms of the `type.size`
 * symbol lookup.
 *
 * So, there is a type-level symbol-table, which can store type oriented
 * information, such as size, alignment, layout, list of members, min/max, and
 * anything else I can think of. Is a set of member functions the "same" as type
 * oriented information? I can't think of a reason why not.
 *
 * Also, also, is the Layout of a type CPU specific? Isn't the concept of size,
 * alignment, and padding shared among all processors? And if so, is that not
 * reason enough to pull layout out of the arch specific subsection of the
 * codegenerator? Well, yes, however, the actual values of a specific types size
 * and alignment are CPU specific are they not? Like, obviously not so much with
 * a u32/u64, as we require a specific number of bits in their representation,
 * which translates to a known size in terms of bytes. The only way this would
 * change would be if a given CPU used more bits per byte. Which is not a common
 * thing that I am aware of, and more of a thing with older architectures.
 *
 */
typedef struct Type {
    TypeKind kind;
    union {
        u8           scalar;
        TupleType    tuple;
        FunctionType function;
    };
} Type;

void type_create_nil(Type *restrict type);
void type_create_bool(Type *restrict type);
void type_create_u8(Type *restrict type);
void type_create_u16(Type *restrict type);
void type_create_u32(Type *restrict type);
void type_create_u64(Type *restrict type);
void type_create_i8(Type *restrict type);
void type_create_i16(Type *restrict type);
void type_create_i32(Type *restrict type);
void type_create_i64(Type *restrict type);
void type_create_tuple(Type *restrict type, TupleType tuple_type);
void type_create_function(Type *restrict type,
                          Type const *result,
                          TupleType   args);
void type_destroy(Type *restrict type);

bool type_equality(Type const *t1, Type const *t2);
bool type_is_scalar(Type const *restrict type);
bool type_is_callable(Type const *restrict type);

bool type_is_index(Type const *restrict type);
bool type_is_indexable(Type const *restrict type);

void print_type(String *restrict string, Type const *restrict type);

#endif // !EXP_IMR_TYPE_H

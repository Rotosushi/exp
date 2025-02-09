

#include "imr/scalar.h"
#include "utility/unreachable.h"

static Scalar scalar_construct(ScalarKind kind, ScalarData data) {
    Scalar scalar = {.kind = kind, .data = data};
    return scalar;
}

Scalar scalar_create() {
    return scalar_construct(SCALAR_UNINITIALIZED, (ScalarData){});
}

/*
Scalar scalar_nil() {
    return scalar_construct(SCALAR_NIL, (ScalarData){.nil = 0});
}

Scalar scalar_bool(bool bool_) {
    return scalar_construct(SCALAR_BOOL, (ScalarData){.bool_ = bool_});
}
*/

Scalar scalar_i64(i64 i64_) {
    return scalar_construct(SCALAR_I64, (ScalarData){.i64_ = i64_});
}

bool scalar_equality(Scalar A, Scalar B) {
    if (A.kind != B.kind) { return false; }
    switch (A.kind) {
    // #NOTE: iff the scalar's are uninitialized, then strictly speaking
    //  they are never to be considered equal. because their values are
    //  unknowable. and thus we cannot know if they are equal or not.
    case SCALAR_UNINITIALIZED: return false;
    case SCALAR_I64:           return A.data.i64_ == B.data.i64_;
    default:                   EXP_UNREACHABLE();
    }
}

void print_scalar(String *buffer, Scalar A) {
    switch (A.kind) {
    case SCALAR_UNINITIALIZED: string_append(buffer, SV("uninitialized"));
    case SCALAR_I64:           string_append_i64(buffer, A.data.i64_);
    default:                   EXP_UNREACHABLE();
    }
}

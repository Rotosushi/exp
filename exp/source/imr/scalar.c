

#include "imr/scalar.h"
#include "utility/unreachable.h"

static Scalar scalar_construct(ScalarKind kind, ScalarData data) {
    Scalar scalar = {.kind = kind, .data = data};
    return scalar;
}

Scalar scalar_create() {
    return scalar_construct(SCALAR_UNINITIALIZED, (ScalarData){.nil = 0});
}

Scalar scalar_nil() {
    return scalar_construct(SCALAR_NIL, (ScalarData){.nil = 0});
}

Scalar scalar_bool(bool bool_) {
    return scalar_construct(SCALAR_BOOL, (ScalarData){.bool_ = bool_});
}

Scalar scalar_i32(i32 i32_) {
    return scalar_construct(SCALAR_I32, (ScalarData){.i32_ = i32_});
}

bool scalar_equality(Scalar A, Scalar B) {
    if (A.kind != B.kind) { return false; }
    switch (A.kind) {
    case SCALAR_UNINITIALIZED: return true;
    case SCALAR_NIL:           return true;
    case SCALAR_BOOL:          return A.data.bool_ == B.data.bool_;
    case SCALAR_I32:           return A.data.i32_ == B.data.i32_;
    default:                   EXP_UNREACHABLE();
    }
}

void print_scalar(FILE *out, Scalar A) {
    switch (A.kind) {
    case SCALAR_UNINITIALIZED: file_write("uninitialized", out);
    case SCALAR_NIL:           file_write("()", out);
    case SCALAR_BOOL:
        A.data.bool_ ? file_write("true", out) : file_write("false", out);
    case SCALAR_I32: file_write_i64(A.data.i32_, out);
    default:         EXP_UNREACHABLE();
    }
}

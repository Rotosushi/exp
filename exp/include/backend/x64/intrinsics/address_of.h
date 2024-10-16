#ifndef EXP_BACKEND_X64_CODEGEN_INTRINSICS_ADDRESS_OF_H
#define EXP_BACKEND_X64_CODEGEN_INTRINSICS_ADDRESS_OF_H

#include "backend/x64/context.h"
#include "backend/x64/location.h"

x64_Address
x64_address_of_global(u64 index, u64 Idx, x64_Context *restrict context);

#endif // EXP_BACKEND_X64_CODEGEN_INTRINSICS_ADDRESS_OF_H

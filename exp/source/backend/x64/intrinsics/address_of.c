
#include "backend/x64/intrinsics/address_of.h"

x64_Address
x64_address_of_global(u64 index, u64 Idx, x64_Context *restrict context) {
  x64_GPR gpr = x64_context_aquire_any_gpr(context, Idx);
  x64_context_append(context,
                     x64_lea(x64_operand_gpr(gpr), x64_operand_label(index)));
  x64_Address global = x64_address_from_gpr(gpr);
  x64_context_release_gpr(context, gpr, Idx);
  return global;
}

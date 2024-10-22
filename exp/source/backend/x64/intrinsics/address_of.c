
#include "backend/x64/intrinsics/address_of.h"

x64_Address
x64_address_of_global(u64 index, u64 Idx, x64_Context *restrict context) {
  SymbolTableElement *element = x64_context_global_symbol(context, index);
  LocalVariable *local =
      x64_context_new_ssa(context, element->type, lifetime_immortal(Idx), Idx);
  x64_Allocation *allocation = x64_context_allocate(context, local, Idx);

  x64_context_append(
      context,
      x64_lea(x64_operand_alloc(allocation), x64_operand_label(index)));

  x64_Address global = x64_address_from_gpr(gpr);
  // we want to call this eventually, because we need to release
  // the finite resource. if we call it right here, then we run
  // the risk of allowing code emitted after this point reusing
  // the gpr we just aquired for another calculation, instead
  // of respecting that it is currently in use. however, if we
  // don't call *release_gpr here, then there is no obvious
  // place to call it, with this particular gpr, effectively using
  // the gpr for the rest of the function body.
  //
  // x64_context_release_gpr(context, gpr, Idx);
  return global;
}

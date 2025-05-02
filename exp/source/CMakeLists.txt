cmake_minimum_required(VERSION 3.20)


set(SOURCE_FILES 
  ${EXP_SOURCE_DIR}/adt/graph.c

  ${EXP_SOURCE_DIR}/analysis/infer_types.c
  ${EXP_SOURCE_DIR}/analysis/infer_lifetimes.c
  
  ${EXP_SOURCE_DIR}/codegen/GAS/directives.c
  ${EXP_SOURCE_DIR}/codegen/IR/codegen.c
  ${EXP_SOURCE_DIR}/codegen/IR/directives.c
  ${EXP_SOURCE_DIR}/codegen/x86/codegen.c
  ${EXP_SOURCE_DIR}/codegen/x86/emit.c
  ${EXP_SOURCE_DIR}/codegen/x86/env/context.c
  ${EXP_SOURCE_DIR}/codegen/x86/env/symbols.c
  ${EXP_SOURCE_DIR}/codegen/x86/intrinsics/copy.c
  ${EXP_SOURCE_DIR}/codegen/x86/intrinsics/get_element_address.c
  ${EXP_SOURCE_DIR}/codegen/x86/intrinsics/load.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/add.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/call.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/div.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/dot.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/let.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/mod.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/mul.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/neg.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/ret.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/sub.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/address.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/allocation.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/allocator.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/bytecode.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/function.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/instruction.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/location.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/operand.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/registers.c

  ${EXP_SOURCE_DIR}/core/analyze.c
  ${EXP_SOURCE_DIR}/core/assemble.c
  ${EXP_SOURCE_DIR}/core/codegen.c
  ${EXP_SOURCE_DIR}/core/compile.c
  ${EXP_SOURCE_DIR}/core/evaluate.c
  ${EXP_SOURCE_DIR}/core/link.c

  ${EXP_SOURCE_DIR}/env/cli_options.c
  ${EXP_SOURCE_DIR}/env/context.c
  ${EXP_SOURCE_DIR}/env/error.c
  ${EXP_SOURCE_DIR}/env/labels.c
  ${EXP_SOURCE_DIR}/env/string_interner.c
  ${EXP_SOURCE_DIR}/env/symbol_table.c
  ${EXP_SOURCE_DIR}/env/type_interner.c
  ${EXP_SOURCE_DIR}/env/constants.c

  ${EXP_SOURCE_DIR}/evaluate/instruction/add.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/call.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/div.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/dot.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/fn.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/let.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/mod.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/mul.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/neg.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/ret.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/sub.c
  ${EXP_SOURCE_DIR}/evaluate/utility/common.c
  ${EXP_SOURCE_DIR}/evaluate/top.c

  ${EXP_SOURCE_DIR}/imr/bytecode.c
  ${EXP_SOURCE_DIR}/imr/function.c
  ${EXP_SOURCE_DIR}/imr/instruction.c
  ${EXP_SOURCE_DIR}/imr/local.c
  ${EXP_SOURCE_DIR}/imr/locals.c
  ${EXP_SOURCE_DIR}/imr/operand.c
  ${EXP_SOURCE_DIR}/imr/tuple.c
  ${EXP_SOURCE_DIR}/imr/type.c
  ${EXP_SOURCE_DIR}/imr/value.c

  ${EXP_SOURCE_DIR}/intrinsics/align_of.c
  ${EXP_SOURCE_DIR}/intrinsics/size_of.c
  ${EXP_SOURCE_DIR}/intrinsics/type_of.c

  ${EXP_SOURCE_DIR}/scanning/lexer.c
  ${EXP_SOURCE_DIR}/scanning/parser.c
)

add_library(exp_common
  ${SOURCE_FILES}
)
target_include_directories(exp_common PUBLIC 
  ${EXP_INCLUDE_DIR}
  ${EXP_LIBEXP_SUPPORT_INCLUDE_DIR}
)
target_compile_options(exp_common PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_common PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp_common PUBLIC exp_support)

add_executable(exp 
  ${EXP_SOURCE_DIR}/main.c
)
target_include_directories(exp PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp PUBLIC exp_common m)

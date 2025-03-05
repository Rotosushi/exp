cmake_minimum_required(VERSION 3.20)


set(SOURCE_FILES 
  ${EXP_SOURCE_DIR}/adt/graph.c

  ${EXP_SOURCE_DIR}/analysis/typecheck.c
  
  ${EXP_SOURCE_DIR}/codegen/directives.c
  ${EXP_SOURCE_DIR}/codegen/lifetimes.c
  ${EXP_SOURCE_DIR}/codegen/x64/codegen.c
  ${EXP_SOURCE_DIR}/codegen/x64/emit.c
  ${EXP_SOURCE_DIR}/codegen/x64/env/context.c
  ${EXP_SOURCE_DIR}/codegen/x64/env/symbols.c
  ${EXP_SOURCE_DIR}/codegen/x64/intrinsics/copy.c
  ${EXP_SOURCE_DIR}/codegen/x64/intrinsics/get_element_address.c
  ${EXP_SOURCE_DIR}/codegen/x64/intrinsics/load.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/add.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/call.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/divide.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/dot.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/load.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/modulus.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/multiply.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/negate.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/return.c
  ${EXP_SOURCE_DIR}/codegen/x64/instruction/subtract.c
  ${EXP_SOURCE_DIR}/codegen/x64/imr/address.c
  ${EXP_SOURCE_DIR}/codegen/x64/imr/allocation.c
  ${EXP_SOURCE_DIR}/codegen/x64/imr/allocator.c
  ${EXP_SOURCE_DIR}/codegen/x64/imr/bytecode.c
  ${EXP_SOURCE_DIR}/codegen/x64/imr/function_body.c
  ${EXP_SOURCE_DIR}/codegen/x64/imr/instruction.c
  ${EXP_SOURCE_DIR}/codegen/x64/imr/location.c
  ${EXP_SOURCE_DIR}/codegen/x64/imr/operand.c
  ${EXP_SOURCE_DIR}/codegen/x64/imr/registers.c

  ${EXP_SOURCE_DIR}/core/analyze.c
  ${EXP_SOURCE_DIR}/core/assemble.c
  ${EXP_SOURCE_DIR}/core/codegen.c
  ${EXP_SOURCE_DIR}/core/compile.c
  ${EXP_SOURCE_DIR}/core/link.c

  ${EXP_SOURCE_DIR}/env/cli_options.c
  ${EXP_SOURCE_DIR}/env/context_options.c
  ${EXP_SOURCE_DIR}/env/context.c
  ${EXP_SOURCE_DIR}/env/error.c
  ${EXP_SOURCE_DIR}/env/labels.c
  ${EXP_SOURCE_DIR}/env/string_interner.c
  ${EXP_SOURCE_DIR}/env/symbol_table.c
  ${EXP_SOURCE_DIR}/env/type_interner.c
  ${EXP_SOURCE_DIR}/env/constants.c

  ${EXP_SOURCE_DIR}/scanning/lexer.c
  ${EXP_SOURCE_DIR}/scanning/parser.c

  ${EXP_SOURCE_DIR}/imr/bytecode.c
  ${EXP_SOURCE_DIR}/imr/function_body.c
  ${EXP_SOURCE_DIR}/imr/instruction.c
  ${EXP_SOURCE_DIR}/imr/operand.c
  ${EXP_SOURCE_DIR}/imr/type.c
  ${EXP_SOURCE_DIR}/imr/value.c

  ${EXP_SOURCE_DIR}/intrinsics/align_of.c
  ${EXP_SOURCE_DIR}/intrinsics/size_of.c
  ${EXP_SOURCE_DIR}/intrinsics/type_of.c
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

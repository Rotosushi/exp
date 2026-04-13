cmake_minimum_required(VERSION 3.20)


execute_process(
  COMMAND git rev-parse --short HEAD 
  OUTPUT_VARIABLE EXP_GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(TIMESTAMP EXP_BUILD_TIME "%Y-%m-%d %H:%M:%S")

set(EXP_VERSION_STRING "")
string(APPEND EXP_VERSION_STRING 
"exp version ("
${EXP_VERSION_MAJOR}
"."
${EXP_VERSION_MINOR}
"."
${EXP_VERSION_PATCH}
") built at ("
${EXP_BUILD_TIME}
") git revision ("
${EXP_GIT_REVISION}
")"
)

configure_file(
  ${EXP_INCLUDE_DIR}/support/config.h.in 
  ${EXP_INCLUDE_DIR}/support/config.h
)

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

  ${EXP_SOURCE_DIR}/support/allocation.c
  ${EXP_SOURCE_DIR}/support/array_growth.c
  ${EXP_SOURCE_DIR}/support/bitset.c
  ${EXP_SOURCE_DIR}/support/cli_option_parser.c
  ${EXP_SOURCE_DIR}/support/constant_string.c
  ${EXP_SOURCE_DIR}/support/hash.c
  ${EXP_SOURCE_DIR}/support/io.c
  ${EXP_SOURCE_DIR}/support/message.c
  ${EXP_SOURCE_DIR}/support/nearest_power.c
  ${EXP_SOURCE_DIR}/support/numeric_conversions.c
  ${EXP_SOURCE_DIR}/support/panic.c
  ${EXP_SOURCE_DIR}/support/process.c
  ${EXP_SOURCE_DIR}/support/random.c
  ${EXP_SOURCE_DIR}/support/scalar.c
  ${EXP_SOURCE_DIR}/support/string_view.c
  ${EXP_SOURCE_DIR}/support/string.c
)

add_library(exp_common
  ${SOURCE_FILES}
)
target_include_directories(exp_common PUBLIC 
  ${EXP_INCLUDE_DIR}
)
target_compile_options(exp_common PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_common PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp_common PUBLIC)

add_executable(exp 
  ${EXP_SOURCE_DIR}/main.c
)
target_include_directories(exp PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp PUBLIC exp_common m)

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
  ${EXP_SOURCE_DIR}/adt/dynamic_bitset_matrix.c
  ${EXP_SOURCE_DIR}/adt/dynamic_bitset.c
  ${EXP_SOURCE_DIR}/adt/sparse_digraph.c

  ${EXP_SOURCE_DIR}/analysis/infer_types.c
  ${EXP_SOURCE_DIR}/analysis/infer_lifetimes.c
  ${EXP_SOURCE_DIR}/analysis/validate.c

  ${EXP_SOURCE_DIR}/codegen/GAS/directives.c

  ${EXP_SOURCE_DIR}/codegen/IR/codegen.c
  ${EXP_SOURCE_DIR}/codegen/IR/directives.c
  ${EXP_SOURCE_DIR}/codegen/IR/intrinsics.c
  ${EXP_SOURCE_DIR}/codegen/IR/target.c

  ${EXP_SOURCE_DIR}/codegen/x86/codegen.c
  ${EXP_SOURCE_DIR}/codegen/x86/function.c
  ${EXP_SOURCE_DIR}/codegen/x86/target.c
  ${EXP_SOURCE_DIR}/codegen/x86/value.c

  ${EXP_SOURCE_DIR}/codegen/x86/env/context.c

  ${EXP_SOURCE_DIR}/codegen/x86/intrinsics/align_of.c
  ${EXP_SOURCE_DIR}/codegen/x86/intrinsics/call.c
  ${EXP_SOURCE_DIR}/codegen/x86/intrinsics/copy.c
  #${EXP_SOURCE_DIR}/codegen/x86/intrinsics/get_element_address.c
  ${EXP_SOURCE_DIR}/codegen/x86/intrinsics/initialize_local.c
  ${EXP_SOURCE_DIR}/codegen/x86/intrinsics/ptr_kind_of.c
  ${EXP_SOURCE_DIR}/codegen/x86/intrinsics/size_of.c

  #${EXP_SOURCE_DIR}/codegen/x86/instruction/add.c
  #${EXP_SOURCE_DIR}/codegen/x86/instruction/call.c
  #${EXP_SOURCE_DIR}/codegen/x86/instruction/div.c
  #${EXP_SOURCE_DIR}/codegen/x86/instruction/dot.c
  #${EXP_SOURCE_DIR}/codegen/x86/instruction/let.c
  #${EXP_SOURCE_DIR}/codegen/x86/instruction/mod.c
  #${EXP_SOURCE_DIR}/codegen/x86/instruction/mul.c
  #${EXP_SOURCE_DIR}/codegen/x86/instruction/neg.c
  ${EXP_SOURCE_DIR}/codegen/x86/instruction/ret.c
  #${EXP_SOURCE_DIR}/codegen/x86/instruction/sub.c

  ${EXP_SOURCE_DIR}/codegen/x86/imr/allocation/location.c

  ${EXP_SOURCE_DIR}/codegen/x86/imr/body/block.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/body/instruction.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/body/operand.c

  ${EXP_SOURCE_DIR}/codegen/x86/imr/local_allocator/allocations.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/local_allocator/incoming_argument_allocator.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/local_allocator/register_allocator.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/local_allocator/register_pool.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/local_allocator/stack_allocator.c

  ${EXP_SOURCE_DIR}/codegen/x86/imr/allocation.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/body.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/function.c
  #${EXP_SOURCE_DIR}/codegen/x86/imr/layout.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/local_allocator.c
  #${EXP_SOURCE_DIR}/codegen/x86/imr/locations.c
  ${EXP_SOURCE_DIR}/codegen/x86/imr/registers.c


  ${EXP_SOURCE_DIR}/env/cli_options.c
  ${EXP_SOURCE_DIR}/env/constants.c
  ${EXP_SOURCE_DIR}/env/context.c
  ${EXP_SOURCE_DIR}/env/error.c
  ${EXP_SOURCE_DIR}/env/frames.c
  ${EXP_SOURCE_DIR}/env/labels.c
  ${EXP_SOURCE_DIR}/env/layout_interner.c
  ${EXP_SOURCE_DIR}/env/stack.c
  ${EXP_SOURCE_DIR}/env/string_interner.c
  ${EXP_SOURCE_DIR}/env/symbol_table.c
  ${EXP_SOURCE_DIR}/env/type_interner.c

  ${EXP_SOURCE_DIR}/evaluate/instruction/add.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/call.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/div.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/dot.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/let.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/mod.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/mul.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/neg.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/ret.c
  ${EXP_SOURCE_DIR}/evaluate/instruction/sub.c
  ${EXP_SOURCE_DIR}/evaluate/utility/common.c
  ${EXP_SOURCE_DIR}/evaluate/top.c
  ${EXP_SOURCE_DIR}/evaluate/evaluate.c

  ${EXP_SOURCE_DIR}/imr/block/instruction.c
  ${EXP_SOURCE_DIR}/imr/block/operand.c
  ${EXP_SOURCE_DIR}/imr/layout/primary.c
  ${EXP_SOURCE_DIR}/imr/layout/tuple.c
  ${EXP_SOURCE_DIR}/imr/type/composite.c
  ${EXP_SOURCE_DIR}/imr/type/function.c
  ${EXP_SOURCE_DIR}/imr/type/primary.c
  ${EXP_SOURCE_DIR}/imr/type/tuple.c
  ${EXP_SOURCE_DIR}/imr/value/function/local.c
  ${EXP_SOURCE_DIR}/imr/value/function/locals.c
  ${EXP_SOURCE_DIR}/imr/value/function.c
  ${EXP_SOURCE_DIR}/imr/value/tuple.c
  ${EXP_SOURCE_DIR}/imr/block.c
  ${EXP_SOURCE_DIR}/imr/layout.c
  ${EXP_SOURCE_DIR}/imr/type.c
  ${EXP_SOURCE_DIR}/imr/value.c

  ${EXP_SOURCE_DIR}/scanning/lexer.c
  ${EXP_SOURCE_DIR}/scanning/parser.c

  ${EXP_SOURCE_DIR}/support/allocation.c
  ${EXP_SOURCE_DIR}/support/arithmetic.c
  ${EXP_SOURCE_DIR}/support/array_growth.c
  ${EXP_SOURCE_DIR}/support/assert.c
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
  ${EXP_SOURCE_DIR}/support/stacktrace.c
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

add_executable(exp
  ${EXP_SOURCE_DIR}/main.c
)
target_include_directories(exp PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp PUBLIC exp_common m)

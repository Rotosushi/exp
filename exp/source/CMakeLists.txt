cmake_minimum_required(VERSION 3.20)

set(EXP_UTILITY_SOURCE_FILES
  ${EXP_SOURCE_DIR}/utility/alloc.c
  ${EXP_SOURCE_DIR}/utility/array_growth.c
  ${EXP_SOURCE_DIR}/utility/bitset.c
  ${EXP_SOURCE_DIR}/utility/constant_string.c
  ${EXP_SOURCE_DIR}/utility/hash.c
  ${EXP_SOURCE_DIR}/utility/int_types.c
  ${EXP_SOURCE_DIR}/utility/io.c
  ${EXP_SOURCE_DIR}/utility/log.c
  ${EXP_SOURCE_DIR}/utility/minmax.c
  ${EXP_SOURCE_DIR}/utility/nearest_power.c
  ${EXP_SOURCE_DIR}/utility/numeric_conversions.c
  ${EXP_SOURCE_DIR}/utility/panic.c
  ${EXP_SOURCE_DIR}/utility/process.c
  ${EXP_SOURCE_DIR}/utility/string_view.c
)

add_library(exp_utility
  ${EXP_UTILITY_SOURCE_FILES}
)
target_include_directories(exp_utility PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp_utility PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_utility PUBLIC ${EXP_LINK_OPTIONS})



set(SOURCE_FILES 
  ${EXP_SOURCE_DIR}/adt/graph.c
  ${EXP_SOURCE_DIR}/adt/string.c

  ${EXP_SOURCE_DIR}/analysis/allocation.c
  ${EXP_SOURCE_DIR}/analysis/finalize.c
  ${EXP_SOURCE_DIR}/analysis/lifetimes.c
  ${EXP_SOURCE_DIR}/analysis/infer_types.c

  ${EXP_SOURCE_DIR}/core/finalize.c
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

  ${EXP_SOURCE_DIR}/frontend/lexer.c
  ${EXP_SOURCE_DIR}/frontend/parser.c

  ${EXP_SOURCE_DIR}/imr/block.c
  ${EXP_SOURCE_DIR}/imr/function.c
  ${EXP_SOURCE_DIR}/imr/instruction.c
  ${EXP_SOURCE_DIR}/imr/lifetime.c
  ${EXP_SOURCE_DIR}/imr/local.c
  ${EXP_SOURCE_DIR}/imr/local_allocator.c
  ${EXP_SOURCE_DIR}/imr/location.c
  ${EXP_SOURCE_DIR}/imr/operand.c
  ${EXP_SOURCE_DIR}/imr/type.c
  ${EXP_SOURCE_DIR}/imr/value.c

  ${EXP_SOURCE_DIR}/intrinsics/align_of.c
  ${EXP_SOURCE_DIR}/intrinsics/size_of.c
  ${EXP_SOURCE_DIR}/intrinsics/type_of.c

  ${EXP_SOURCE_DIR}/targets/ir/emit.c

  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/add.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/call.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/idiv.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/imul.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/lea.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/mov.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/neg.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/pop.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/push.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/ret.c
  ${EXP_SOURCE_DIR}/targets/x86_64/instructions/sub.c
  ${EXP_SOURCE_DIR}/targets/x86_64/address.c
  ${EXP_SOURCE_DIR}/targets/x86_64/as_directives.c
  ${EXP_SOURCE_DIR}/targets/x86_64/emit.c
  ${EXP_SOURCE_DIR}/targets/x86_64/mnemonic.c
  ${EXP_SOURCE_DIR}/targets/x86_64/operand.c
  ${EXP_SOURCE_DIR}/targets/x86_64/registers.c
)

add_library(exp_common
  ${SOURCE_FILES}
)
target_include_directories(exp_common PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp_common PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_common PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp_common PUBLIC exp_utility)

add_executable(exp 
  ${EXP_SOURCE_DIR}/main.c
)
target_include_directories(exp PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp PUBLIC exp_common m)

cmake_minimum_required(VERSION 3.20)

set(SOURCE_FILES 
  ${EXP_SOURCE_DIR}/adt/graph.c
  ${EXP_SOURCE_DIR}/adt/string.c

  ${EXP_SOURCE_DIR}/backend/directives.c
  ${EXP_SOURCE_DIR}/backend/lifetimes.c

  ${EXP_SOURCE_DIR}/backend/x64/intrinsics/copy.c

  ${EXP_SOURCE_DIR}/backend/x64/allocation.c
  ${EXP_SOURCE_DIR}/backend/x64/allocator.c
  ${EXP_SOURCE_DIR}/backend/x64/bytecode.c
  ${EXP_SOURCE_DIR}/backend/x64/codegen.c
  ${EXP_SOURCE_DIR}/backend/x64/context.c
  ${EXP_SOURCE_DIR}/backend/x64/emit.c
  ${EXP_SOURCE_DIR}/backend/x64/function_body.c
  ${EXP_SOURCE_DIR}/backend/x64/instruction.c
  ${EXP_SOURCE_DIR}/backend/x64/location.c
  ${EXP_SOURCE_DIR}/backend/x64/operand.c
  ${EXP_SOURCE_DIR}/backend/x64/registers.c
  ${EXP_SOURCE_DIR}/backend/x64/symbols.c

  ${EXP_SOURCE_DIR}/core/assemble.c
  ${EXP_SOURCE_DIR}/core/codegen.c
  ${EXP_SOURCE_DIR}/core/compile.c
  ${EXP_SOURCE_DIR}/core/link.c
  ${EXP_SOURCE_DIR}/core/typecheck.c

  ${EXP_SOURCE_DIR}/env/cli_options.c
  ${EXP_SOURCE_DIR}/env/constants.c
  ${EXP_SOURCE_DIR}/env/context_options.c
  ${EXP_SOURCE_DIR}/env/context.c
  ${EXP_SOURCE_DIR}/env/error.c
  ${EXP_SOURCE_DIR}/env/global_labels.c
  ${EXP_SOURCE_DIR}/env/string_interner.c
  ${EXP_SOURCE_DIR}/env/symbol_table.c
  ${EXP_SOURCE_DIR}/env/type_interner.c

  ${EXP_SOURCE_DIR}/frontend/lexer.c
  ${EXP_SOURCE_DIR}/frontend/parser.c

  ${EXP_SOURCE_DIR}/imr/bytecode.c
  ${EXP_SOURCE_DIR}/imr/function_body.c
  ${EXP_SOURCE_DIR}/imr/instruction.c
  ${EXP_SOURCE_DIR}/imr/operand.c
  ${EXP_SOURCE_DIR}/imr/type.c
  ${EXP_SOURCE_DIR}/imr/value.c

  ${EXP_SOURCE_DIR}/intrinsics/align_of.c
  ${EXP_SOURCE_DIR}/intrinsics/size_of.c
  ${EXP_SOURCE_DIR}/intrinsics/type_of.c

  ${EXP_SOURCE_DIR}/utility/alloc.c
  ${EXP_SOURCE_DIR}/utility/array_growth.c
  ${EXP_SOURCE_DIR}/utility/hash.c
  ${EXP_SOURCE_DIR}/utility/io.c
  ${EXP_SOURCE_DIR}/utility/log.c
  ${EXP_SOURCE_DIR}/utility/minmax.c
  ${EXP_SOURCE_DIR}/utility/nearest_power.c
  ${EXP_SOURCE_DIR}/utility/numeric_conversions.c
  ${EXP_SOURCE_DIR}/utility/panic.c
  ${EXP_SOURCE_DIR}/utility/process.c
  ${EXP_SOURCE_DIR}/utility/string_view.c
  
)

add_library(exp_common
  ${SOURCE_FILES}
)
target_include_directories(exp_common PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp_common PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_common PUBLIC ${EXP_LINK_OPTIONS})

add_executable(exp 
  ${EXP_SOURCE_DIR}/main.c
)
target_include_directories(exp PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp PUBLIC exp_common m)

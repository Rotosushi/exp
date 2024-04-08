cmake_minimum_required(VERSION 3.20)

set(SOURCE_FILES 
  ${EXP_SOURCE_DIR}/backend/emit_x64_linux_assembly.c

  ${EXP_SOURCE_DIR}/core/compile.c
  ${EXP_SOURCE_DIR}/core/interpret.c

  ${EXP_SOURCE_DIR}/env/context_options.c
  ${EXP_SOURCE_DIR}/env/context.c
  ${EXP_SOURCE_DIR}/env/error.c
  ${EXP_SOURCE_DIR}/env/string_interner.c
  ${EXP_SOURCE_DIR}/env/symbol_table.c
  ${EXP_SOURCE_DIR}/env/type_interner.c

  ${EXP_SOURCE_DIR}/frontend/lexer.c
  ${EXP_SOURCE_DIR}/frontend/parser.c

  ${EXP_SOURCE_DIR}/imr/bytecode.c
  ${EXP_SOURCE_DIR}/imr/constants.c
  ${EXP_SOURCE_DIR}/imr/function.c
  ${EXP_SOURCE_DIR}/imr/registers.c
  ${EXP_SOURCE_DIR}/imr/stack.c
  ${EXP_SOURCE_DIR}/imr/type.c
  ${EXP_SOURCE_DIR}/imr/value.c

  ${EXP_SOURCE_DIR}/intrinsics/alignment.c
  ${EXP_SOURCE_DIR}/intrinsics/size.c

  ${EXP_SOURCE_DIR}/utility/cli_options.c
  ${EXP_SOURCE_DIR}/utility/io.c
  ${EXP_SOURCE_DIR}/utility/log.c
  ${EXP_SOURCE_DIR}/utility/minmax.c
  ${EXP_SOURCE_DIR}/utility/nearest_power.c
  ${EXP_SOURCE_DIR}/utility/numbers_to_string.c
  ${EXP_SOURCE_DIR}/utility/panic.c
  ${EXP_SOURCE_DIR}/utility/path.c
  ${EXP_SOURCE_DIR}/utility/process.c
  ${EXP_SOURCE_DIR}/utility/string_hash.c
  ${EXP_SOURCE_DIR}/utility/string_view.c
  ${EXP_SOURCE_DIR}/utility/string.c
)

add_library(exp_common
  ${SOURCE_FILES}
)
target_include_directories(exp_common PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp_common PUBLIC ${WARNINGS})

add_executable(exp 
  ${EXP_SOURCE_DIR}/main.c
)
target_include_directories(exp PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp PUBLIC ${WARNINGS})
target_link_libraries(exp PUBLIC exp_common m)
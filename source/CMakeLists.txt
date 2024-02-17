cmake_minimum_required(VERSION 3.20)

set(SOURCE_FILES 
  ${EXP_SOURCE_DIR}/frontend/lexer.c

  ${EXP_SOURCE_DIR}/utility/fileio.c
  ${EXP_SOURCE_DIR}/utility/log_message.c
  ${EXP_SOURCE_DIR}/utility/numbers_to_string.c
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
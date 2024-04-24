cmake_minimum_required(VERSION 3.20)

add_test (NAME "smoke test" COMMAND exp -v)

create_test_sourcelist(Tests exp_tests.c
  cli_options_tests.c
  constants_tests.c
  control_flow_graph_tests.c
  graph_tests.c
  instruction_tests.c
  lexer_tests.c
  lifetime_intervals_tests.c
  number_conversion_tests.c
  parse_tests.c
  string_interner_tests.c
  string_tests.c
  symbol_table_tests.c
  type_interner_tests.c
)

add_executable (exp_tests ${Tests})
target_include_directories(exp_tests PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp_tests PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_tests PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp_tests PUBLIC exp_common m)


set (TestsToRun ${Tests})
remove (TestsToRun exp_tests.c)

foreach (test ${TestsToRun})
  get_filename_component (TestName ${test} NAME_WE)
  add_test (NAME ${TestName} COMMAND exp_tests ${TestName})
endforeach()
cmake_minimum_required(VERSION 3.20)

add_test (NAME "smoke test" COMMAND exp)

create_test_sourcelist(Tests 
  CommonTests.c
  number_conversion.c
  lexer_tests.c
  string_tests.c
  
)

add_executable (CommonTests ${Tests})
target_include_directories(CommonTests PUBLIC ${EXP_INCLUDE_DIR})
target_link_libraries(CommonTests PUBLIC exp_common m)

set (TestsToRun ${Tests})
remove (TestsToRun CommonTests.c)

foreach (test ${TestsToRun})
  get_filename_component (TestName ${test} NAME_WE)
  add_test (NAME ${TestName} COMMAND CommonTests ${TestName})
endforeach()
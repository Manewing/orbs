function(add_orbs_unittest)

set(options "")
set(oneValueArgs NAME RENAME)
set(multiValueArgs SOURCES INCLUDES)

cmake_parse_arguments(
  ARGS
  "${options}"
  "${oneValueArgs}"
  "${multiValueArgs}"
  ${ARGN}
)

set(TARGET test_${ARGS_NAME})

add_executable(${TARGET}
  ${ARGS_SOURCES}
)

add_test(unittest_${ARGS_NAME} ${TARGET} ${CMAKE_SOURCE_DIR})

target_link_libraries(${TARGET}
  ${GTEST_LIBRARIES}
  ${GTEST_MAIN_LIBRARY}
  ${ORBS_LIBRARIES}
)

target_include_directories(${TARGET}
  PUBLIC ${ARGS_INCLUDES}
)

endfunction()

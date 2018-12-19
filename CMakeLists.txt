cmake_minimum_required(VERSION 3.5)
project(orbs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(cmake/AddOrbsUnitTest.cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O3 -ggdb"
)

set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -O3 -ggdb"
)

set(ORBS_LIBRARIES
  pthread
  readline
)

# for apple we have to manually add argp
if (APPLE)
  find_package(argp REQUIRED)
  include_directories(SYSTEM ${ARGP_INCLUDE_PATH})
  set(ORBS_LIBRARIES
    ${ORBS_LIBRARIES}
    ${ARGP_LIBRARIES}
  )
endif()

add_subdirectory(orbs)

option(BUILD_TESTS "If to build unit tests (need google-test)" OFF)
if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

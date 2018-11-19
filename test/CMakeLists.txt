find_package(GTest "1.8.0" REQUIRED)
find_package(GMock "1.8.0" REQUIRED)
include_directories(SYSTEM ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

# orb basics test
set(SOURCES
  test_orb.cpp
  ../orbs/src/orb.c
  ../orbs/src/map.c
  ../orbs/src/config.c
  ../orbs/src/mm.c
  ../orbs/src/stats.c
  ../orbs/src/list.c
)

add_orbs_unittest(
  NAME orbs
  SOURCES ${SOURCES}
  INCLUDES ../orbs/inc/
)

# orb instructions test
set(SOURCES
  test_orb_instr.cpp
  ../orbs/src/orb.c
  ../orbs/src/map.c
  ../orbs/src/config.c
  ../orbs/src/mm.c
  ../orbs/src/stats.c
  ../orbs/src/list.c
)

add_orbs_unittest(
  NAME orbs_instr
  SOURCES ${SOURCES}
  INCLUDES ../orbs/inc/
)
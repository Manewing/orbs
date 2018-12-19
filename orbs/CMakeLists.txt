set(TARGET orbs)

set(HEADERS
  inc/config.h
  inc/config_reader.h
  inc/defines.h
  inc/list.h
  inc/map.h
  inc/mm.h
  inc/orb.h
  inc/orb_instr.h
  inc/shell.h
  inc/stats.h
)

set(SOURCES
  src/config.c
  src/config_reader.c
  src/list.c
  src/main.c
  src/map.c
  src/mm.c
  src/orb.c
  src/shell.c
  src/stats.c
)

add_executable(${TARGET}
  ${SOURCES}
  ${HEADERS}
)

target_link_libraries(${TARGET}
  ${ORBS_LIBRARIES}
)

target_include_directories(${TARGET} PUBLIC inc)

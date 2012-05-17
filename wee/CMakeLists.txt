set(CMAKE_C_FLAGS "-Wall -Wno-unused -g -std=c99")
set(SRCS
  "list.c"
  "hash.c"
  "message.c"
  "object_space.c"
  "object.c"
  "slot.c"
  "str.c"
  "num.c"
  "parser.c"
)
set(PEG_GRAMMAR "${CMAKE_CURRENT_SOURCE_DIR}/acute.peg")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/parser.c
  COMMAND ${PROJECT_BINARY_DIR}/peg/leg -o ${CMAKE_CURRENT_SOURCE_DIR}/parser.c ${PEG_GRAMMAR}
  DEPENDS leg
)

add_library(wee SHARED ${SRCS})
add_dependencies(wee leg)

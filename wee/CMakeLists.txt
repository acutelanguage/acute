set(CMAKE_C_FLAGS "-g -std=c99")
set(SRCS
  "list.c"
  "hash.c"
  "message.c"
  "object.c"
  "slot.c"
)

add_library(wee SHARED ${SRCS})

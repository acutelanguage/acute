set(CMAKE_C_FLAGS "-g -std=c99")
set(SRCS
  "list.c"
  "message.c"
  "object.c"
  "sparse_array.c"
)

add_library(wee SHARED ${SRCS})

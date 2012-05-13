set(CMAKE_C_FLAGS "-g -std=c99")
set(SRCS
  "sparse_array.c"
)

add_library(wee SHARED ${SRCS})

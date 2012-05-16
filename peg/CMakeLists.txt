set(CMAKE_C_FLAGS "-g -std=c99 -DNDEBUG")
set(SRCS
  "leg.c"
  "tree.c"
  "compile.c"
)

add_executable(leg ${SRCS})

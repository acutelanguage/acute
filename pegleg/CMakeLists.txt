set(CMAKE_CXX_FLAGS "-g -Wall -DNDEBUG -O3")
set(COMMON_SRCS
  "compile.c"
  "tree.c"
)
set(LEG_SRCS ${COMMON_SRCS} "leg.c")
set(PEG_SRCS ${COMMON_SRCS} "peg.c")

add_executable(leg ${LEG_SRCS})
add_executable(peg ${PEG_SRCS})

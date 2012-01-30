set(CMAKE_CXX_FLAGS "-g -std=c++0x -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")
set(SRCS
  "symtab.cpp"
  "gc.cpp"
  "object.cpp"
  "message.cpp"
  "array.cpp"
  "integer.cpp"
  "string.cpp"
  "nil.cpp"
  "object_space.cpp"
  "block.cpp"
)

add_library(wee SHARED ${SRCS})

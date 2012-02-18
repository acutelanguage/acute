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
  "parser.cpp"
)
#set(PARSER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/acute.peg")

#add_custom_command(
#  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
#  COMMAND ${PROJECT_BINARY_DIR}/pegleg/leg -o ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp ${PARSER_SRC}
#  DEPENDS leg
#)
#list(APPEND SRCS "${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp")

add_library(wee SHARED ${SRCS})

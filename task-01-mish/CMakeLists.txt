cmake_minimum_required(VERSION 3.22)
project(task-1-mish)

# Find bison and flex and set CMP0098 policy to new to use binary dir 
find_package(FLEX)
find_package(BISON)
cmake_policy(SET CMP0098 NEW)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(EXEC_SOURCES
  src/main.cc
  src/builtin_commands.cc
)

# bison_target(parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.y.cc DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.y.hpp
#     COMPILE_FLAGS --debug)
# flex_target(scanner src/scanner.l scanner.l.cc DEFINES_FILE include/scanner.l.hpp)
# add_flex_bison_dependency(scanner parser)

# add_library(parserlexer STATIC ${BISON_parser_OUTPUTS} ${FLEX_scanner_OUTPUTS} ${LIB_SOURCES})

add_executable(mish ${EXEC_SOURCES})
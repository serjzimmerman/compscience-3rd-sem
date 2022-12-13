cmake_minimum_required(VERSION 3.22)
project(task-7-copy-server)

set(CMAKE_VERBOSE_MAKEFILE ON)

option(INSOURCEBUILD OFF)

if((${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR}) AND NOT ${INSOURCEBUILD})
  message(FATAL_ERROR "In-source building disabled. Provide -DINSOURCEBUILD option if you are sure about it.")
endif()

include(FetchContent)

find_package(GTest)
set(NOGTEST OFF CACHE BOOL "Disable GoogleTest")

enable_testing()
if(NOT NOGTEST AND GTEST_FOUND AND CMAKE_MINOR_VERSION GREATER_EQUAL 20)
  set(ENABLE_GTEST ON)
else()
  message(WARNING "Google Test disabled")
endif()

set(NOLINT ON CACHE BOOL "Disable clang-tidy")

if(NOT ${NOLINT})
find_program(CLANG_TIDY_COMMAND clang-tidy)
if(CLANG_TIDY_COMMAND)

# Clang-tidy for linting
set(CMAKE_CXX_CLANG_TIDY 
  clang-tidy;
  -checks=google-*,cppcoreguidelines-*
)
set(CMAKE_C_CLANG_TIDY 
  clang-tidy;
  -checks=google-*,cppcoreguidelines-*
)

else()
message(WARNING "Unable to find clang-tidy, linter disabled")
endif()

else()
message(WARNING "-DNOLINT option provided, linter disabled")
endif()

set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(BASH_PROGRAM bash)

# Add flags for GNU sampling profiler gprof in Debug build and RelWithDebInfo
option(PROFILE OFF)
if (PROFILE)
  add_compile_options(-pg)
  add_link_options(-pg)
endif()

option(SANITIZE OFF)
if (SANITIZE)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address -fno-omit-frame-pointer)
endif()

set(CMAKE_CXX_STANDARD 20)
find_package(Boost COMPONENTS program_options)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

bison_target(parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.y.cc DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/bison_command_parser.hpp)
flex_target(scanner src/scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.l.cc DEFINES_FILE)
add_flex_bison_dependency(scanner parser)

set(SERVER_SOURCES
  src/server.cc
  ${BISON_parser_OUTPUTS}
  ${FLEX_scanner_OUTPUTS}
)

add_executable(server ${SERVER_SOURCES})
target_link_libraries(server Boost::program_options)
target_include_directories(server PUBLIC include ${CMAKE_CURRENT_BINARY_DIR})
cmake_minimum_required(VERSION 3.16)
project(LALR1ParserGenerator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Core library
set(LALR1_SOURCES
    token.cpp
    lexer.cpp
    symbol.cpp
    grammar.cpp
    lr_items.cpp
    parse_table.cpp
    parser.cpp
    g4_parser.cpp
    cpp_codegen.cpp
)

add_library(lalr1_lib STATIC ${LALR1_SOURCES})

# Test executables
add_executable(test_basic
    test_basic.cpp
)
target_link_libraries(test_basic lalr1_lib)

# Full LALR(1) parser test
add_executable(test_lalr1
    test_lalr1.cpp
)
target_link_libraries(test_lalr1 lalr1_lib)

# Safe test without parser
add_executable(test_safe
    test_safe.cpp
)
target_link_libraries(test_safe lalr1_lib)

# Command-line tool for G4 grammar processing
add_executable(lalr1_tool
    lalr1_tool.cpp
)
target_link_libraries(lalr1_tool lalr1_lib)

# Install rules
install(TARGETS lalr1_lib DESTINATION lib)
install(FILES
    token.h
    lexer.h
    symbol.h
    grammar.h
    lr_items.h
    parse_table.h
    parser.h
    g4_parser.h
    DESTINATION include
)
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
)

add_library(lalr1_lib STATIC ${LALR1_SOURCES})

# Test executable
add_executable(test_basic
    test_basic.cpp
)
target_link_libraries(test_basic lalr1_lib)

# Install rules
install(TARGETS lalr1_lib DESTINATION lib)
install(FILES
    token.h
    lexer.h
    symbol.h
    grammar.h
    DESTINATION include
)
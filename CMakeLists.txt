cmake_minimum_required(VERSION 3.20)
project(IONet VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(IONET_BUILD_TESTS "Build unit tests" ON)
option(IONET_COVERAGE "Enable coverage reporting" OFF)

add_library(ionet STATIC
    src/core/ByteBuffer.cpp
    src/schema/Schema.cpp
    src/codec/Decoder.cpp
)

target_include_directories(ionet PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(ionet PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

if(IONET_COVERAGE)
    target_compile_options(ionet PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(ionet PUBLIC --coverage)
endif()

if(IONET_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
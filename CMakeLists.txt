cmake_minimum_required(VERSION 3.20)
project(IONet VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(IONET_BUILD_TESTS "Build unit tests" ON)
option(IONET_COVERAGE "Enable coverage reporting" OFF)

# Dependencies via FetchContent
include(FetchContent)

# yaml-cpp
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(yaml-cpp)

# nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Library
add_library(ionet STATIC
    src/core/ByteBuffer.cpp
    src/schema/Schema.cpp
    src/schema/SchemaSource.cpp
    src/schema/SchemaParserBase.cpp
    src/schema/YamlSchemaParser.cpp
    src/schema/JsonSchemaParser.cpp
    src/schema/SchemaLoader.cpp
)

target_include_directories(ionet PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(ionet PUBLIC
    yaml-cpp::yaml-cpp
    nlohmann_json::nlohmann_json
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
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

# Library type option
option(IONET_BUILD_SHARED "Build IONet as a shared library" ON)

set(IONET_SOURCES
    src/core/ByteBuffer.cpp
    src/schema/Schema.cpp
    src/schema/SchemaSource.cpp
    src/schema/SchemaParserBase.cpp
    src/schema/YamlSchemaParser.cpp
    src/schema/JsonSchemaParser.cpp
    src/schema/SchemaLoader.cpp
    src/codec/DecodedPacket.cpp
    src/codec/Decoder.cpp
)

if(IONET_BUILD_SHARED)
    add_library(ionet SHARED ${IONET_SOURCES})
else()
    add_library(ionet STATIC ${IONET_SOURCES})
endif()

target_include_directories(ionet PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(ionet PRIVATE
    yaml-cpp::yaml-cpp
    nlohmann_json::nlohmann_json
)

target_compile_options(ionet PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

# Install rules
include(GNUInstallDirs)
install(TARGETS ionet
    EXPORT ionetTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/ionet DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ionetTargets
    FILE ionetTargets.cmake
    NAMESPACE ionet::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ionet
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ionetConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ionetConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ionetConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ionet
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ionetConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ionetConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ionet
)

if(IONET_COVERAGE)
    target_compile_options(ionet PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(ionet PUBLIC --coverage)
endif()

if(IONET_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Add 2heartbeats example
add_subdirectory(examples/2heartbeats)
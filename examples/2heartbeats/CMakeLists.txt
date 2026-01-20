cmake_minimum_required(VERSION 3.20)

project(2heartbeats_example LANGUAGES CXX)

add_executable(2heartbeats
    src/main.cpp
)

target_include_directories(2heartbeats PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(2heartbeats PRIVATE ionet)

# Optionally, copy config/schema on build
add_custom_command(TARGET 2heartbeats POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/config ${CMAKE_CURRENT_BINARY_DIR}/config
)

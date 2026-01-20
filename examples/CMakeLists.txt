add_executable(2heartbeats
    2heartbeats/main.cpp
)
target_include_directories(2heartbeats PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(2heartbeats PRIVATE ionet)

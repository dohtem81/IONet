include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(ionet_tests
    test_endian.cpp
    test_bytebuffer.cpp
    test_schema.cpp
    test_schema_loader.cpp
    test_decoder.cpp
    test_encoder.cpp      # Add test for Encoder
    #test_packet.cpp       # Add test for Packet
    #test_field.cpp        # Add test for Field (if applicable)
)

target_link_libraries(ionet_tests PRIVATE ionet Catch2::Catch2WithMain)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

include(Catch)
catch_discover_tests(ionet_tests)
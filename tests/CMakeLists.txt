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
)

target_link_libraries(ionet_tests PRIVATE ionet Catch2::Catch2WithMain)

if(IONET_COVERAGE)
    target_compile_options(ionet_tests PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(ionet_tests PRIVATE --coverage)
endif()

include(Catch)
catch_discover_tests(ionet_tests)
cmake_minimum_required(VERSION 3.16)
project(RailDiagGateway VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_SANITIZERS "Enable Address and UndefinedBehavior sanitizers" OFF)
option(BUILD_TESTING "Build the test suite" ON)

if(ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(raildiag_core STATIC
    src/telemetry.cpp
    src/diagnostic.cpp
    src/sequence_guard.cpp
    src/logger.cpp
    src/server.cpp
)

target_include_directories(raildiag_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(raildiag_core PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
)

find_package(Threads REQUIRED)
target_link_libraries(raildiag_core PUBLIC Threads::Threads)

add_executable(raildiag-gateway src/main.cpp)
target_link_libraries(raildiag-gateway PRIVATE raildiag_core)
target_compile_options(raildiag-gateway PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
)

install(TARGETS raildiag-gateway RUNTIME DESTINATION bin)

if(BUILD_TESTING)
    include(CTest)
    find_package(GTest QUIET)

    if(GTest_FOUND)
        add_executable(raildiag-unit-tests
            tests/test_telemetry.cpp
            tests/test_diagnostic.cpp
            tests/test_sequence_guard.cpp
        )
        target_link_libraries(raildiag-unit-tests PRIVATE
            raildiag_core
            GTest::gtest
            GTest::gtest_main
        )
        target_compile_options(raildiag-unit-tests PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wshadow
        )
        add_test(NAME UnitTests COMMAND raildiag-unit-tests)
    endif()
endif()

cmake_minimum_required(VERSION 3.21)
project(TextCompareLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(TextCompareLib STATIC
    src/TextCompare.cpp
)

target_include_directories(TextCompareLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(TextCompareDemo
    demo/main.cpp
)

target_link_libraries(TextCompareDemo
    PRIVATE
        TextCompareLib
)

add_executable(AntipatternDemo
    antipattern/main.cpp
)

find_package(GTest CONFIG REQUIRED)
enable_testing()

add_executable(TextCompareTests
    tests/TextCompareTests.cpp
)

target_link_libraries(TextCompareTests
    PRIVATE
        TextCompareLib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(TextCompareTests)

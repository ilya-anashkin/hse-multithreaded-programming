cmake_minimum_required(VERSION 3.16)
project(futex_mutex_hw LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
enable_testing()

add_executable(tests tests/test_futex_mutex.cpp)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tests PRIVATE GTest::gtest_main pthread)

include(GoogleTest)
gtest_discover_tests(tests)

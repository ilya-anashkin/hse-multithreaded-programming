cmake_minimum_required(VERSION 3.16)

project(hw2_processes_and_threads LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(apply_function INTERFACE)
target_include_directories(apply_function INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)
target_link_libraries(apply_function INTERFACE Threads::Threads)

add_executable(apply_function_demo src/main.cpp)
target_link_libraries(apply_function_demo PRIVATE apply_function)

find_package(GTest REQUIRED)
add_executable(apply_function_tests tests/apply_function_test.cpp)
target_link_libraries(apply_function_tests PRIVATE apply_function GTest::gtest GTest::gtest_main)

include(CTest)
include(GoogleTest)
gtest_discover_tests(apply_function_tests)

find_package(benchmark QUIET)
if(benchmark_FOUND)
  add_executable(apply_function_bench bench/apply_function_bench.cpp)
  target_link_libraries(apply_function_bench PRIVATE apply_function benchmark::benchmark)

  add_custom_target(
    bench
    COMMAND $<TARGET_FILE:apply_function_bench> --benchmark_min_time=0.2s
    DEPENDS apply_function_bench
    USES_TERMINAL
  )

  add_custom_target(
    run
    COMMAND $<TARGET_FILE:apply_function_demo>
    COMMAND $<TARGET_FILE:apply_function_tests>
    COMMAND $<TARGET_FILE:apply_function_bench> --benchmark_min_time=0.2s
    DEPENDS apply_function_demo apply_function_tests apply_function_bench
    USES_TERMINAL
  )
else()
  add_custom_target(
    bench
    COMMAND ${CMAKE_COMMAND} -E echo "google-benchmark was not found. Install benchmark and reconfigure."
    COMMAND ${CMAKE_COMMAND} -E false
    USES_TERMINAL
  )

  add_custom_target(
    run
    COMMAND $<TARGET_FILE:apply_function_demo>
    COMMAND $<TARGET_FILE:apply_function_tests>
    COMMAND ${CMAKE_COMMAND} -E echo "google-benchmark was not found. Install benchmark and reconfigure."
    COMMAND ${CMAKE_COMMAND} -E false
    DEPENDS apply_function_demo apply_function_tests
    USES_TERMINAL
  )
endif()

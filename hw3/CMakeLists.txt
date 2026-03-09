cmake_minimum_required(VERSION 3.25)

project(buffered_channel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_TESTS "Build tests" OFF)
option(ENABLE_BENCHMARK "Build benchmark" OFF)

find_package(Threads REQUIRED)

add_executable(buffered_channel_app main.cpp)
target_link_libraries(buffered_channel_app PRIVATE Threads::Threads)

if (ENABLE_TESTS OR ENABLE_BENCHMARK)
  include(FetchContent)
endif()

if (ENABLE_TESTS)
  FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.17.0
  )
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_executable(buffered_channel_test test.cpp)
  target_link_libraries(buffered_channel_test PRIVATE Threads::Threads GTest::gtest GTest::gtest_main)
  add_test(NAME buffered_channel_test COMMAND buffered_channel_test)
endif()

if (ENABLE_BENCHMARK)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
      googlebenchmark
      GIT_REPOSITORY https://github.com/google/benchmark.git
      GIT_TAG v1.9.5
  )
  FetchContent_MakeAvailable(googlebenchmark)

  add_executable(buffered_channel_benchmark benchmark.cpp)
  target_link_libraries(buffered_channel_benchmark PRIVATE Threads::Threads benchmark::benchmark)
endif()

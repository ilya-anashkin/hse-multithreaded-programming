cmake_minimum_required(VERSION 3.16)
project(hw5_ipc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ipc_queue ipc_queue.cpp)
target_include_directories(ipc_queue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(producer_demo producer_demo.cpp)
target_link_libraries(producer_demo PRIVATE ipc_queue)

add_executable(consumer_demo consumer_demo.cpp)
target_link_libraries(consumer_demo PRIVATE ipc_queue)

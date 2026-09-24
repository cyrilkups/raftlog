cmake_minimum_required(VERSION 3.20)
project(RaftLog CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

enable_testing()

add_executable(raftlog_tests tests/smoke_test.cpp tests/log_test.cpp)
target_include_directories(raftlog_tests PRIVATE src)
target_link_libraries(raftlog_tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(raftlog_tests)

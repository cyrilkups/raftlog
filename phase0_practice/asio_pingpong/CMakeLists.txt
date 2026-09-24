cmake_minimum_required(VERSION 3.20)
project(asio_pingpong CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_executable(pingpong_server server.cpp)
target_link_libraries(pingpong_server PRIVATE Boost::system Threads::Threads)

add_executable(pingpong_client client.cpp)
target_link_libraries(pingpong_client PRIVATE Boost::system Threads::Threads)

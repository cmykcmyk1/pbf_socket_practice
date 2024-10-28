cmake_minimum_required(VERSION 3.5)

project(pbf_socket_practice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(utils)

set(SRC_FILES utils/tcp_socket.cpp utils/tcp_server.cpp)

add_executable(server server.cpp ${SRC_FILES})
add_executable(client client.cpp ${SRC_FILES})

cmake_minimum_required(VERSION 3.13)
project(asio)

set(CMAKE_CXX_STANDARD 14)
link_directories(/usr/local/lib)
add_executable(asio main.cpp examples_buffer.h)
target_link_libraries(asio pthread)
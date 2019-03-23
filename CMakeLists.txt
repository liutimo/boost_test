cmake_minimum_required(VERSION 3.13)
project(boost_asio_example)

set(CMAKE_CXX_STANDARD 14)

link_directories(/usr/local/lib)
add_executable(boost_asio_example main.cpp alloction.h)
target_link_libraries(boost_asio_example pthread)
cmake_minimum_required(VERSION 3.20)

project(compile)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch")

add_executable(compile src/main.cpp)

cmake_minimum_required(VERSION 3.7...3.27)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

project(numbvm LANGUAGES CXX)

add_executable(numbvm src/main.cpp src/vm.cpp)

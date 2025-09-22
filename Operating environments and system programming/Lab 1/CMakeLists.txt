cmake_minimum_required(VERSION 3.10)
project(MyProject)


set(CMAKE_CXX_STANDARD 17)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(MyProject src/main.cpp)

target_include_directories(MyProject PRIVATE include)
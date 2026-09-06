cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)          
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Lab1)

add_subdirectory(external/fltk)

add_executable(Lab1
    src/main.cpp
)

target_link_libraries(Lab1 PRIVATE fltk)
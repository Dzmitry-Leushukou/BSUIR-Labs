cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Lab1)

add_subdirectory(external/fltk)

add_executable(Lab1
    src/main.cpp
)

target_include_directories(Lab1 PRIVATE
    ${CMAKE_BINARY_DIR}/external/fltk
    ${CMAKE_SOURCE_DIR}/external/fltk
)

target_link_libraries(Lab1 PRIVATE fltk)
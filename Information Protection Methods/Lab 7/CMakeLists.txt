cmake_minimum_required(VERSION 3.20)

project(Lab7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Lab7 src/main.cpp)

if (WIN32)
    target_link_libraries(Lab7 PRIVATE bcrypt)
endif()

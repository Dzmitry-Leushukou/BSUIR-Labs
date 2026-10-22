cmake_minimum_required(VERSION 3.20)

project(Lab4 LANGUAGES CXX)

if (NOT WIN32)
    message(FATAL_ERROR "Lab4 is a Windows-only project.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Lab4 src/main.cpp)
target_link_libraries(Lab4 PRIVATE bcrypt)

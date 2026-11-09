cmake_minimum_required(VERSION 3.20)

project(Lab7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Lab7
    src/main.cpp
    src/EllipticCurve.cpp
    src/ElGamal.cpp
    src/CipherFile.cpp
)

if (WIN32)
    target_link_libraries(Lab7 PRIVATE bcrypt)
endif()

if (MSVC)
    target_compile_options(Lab7 PRIVATE /W4)
else()
    target_compile_options(Lab7 PRIVATE -Wall -Wextra -Wpedantic)
endif()

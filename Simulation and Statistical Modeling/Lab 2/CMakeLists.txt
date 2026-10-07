cmake_minimum_required(VERSION 3.20)
project(lab2_simulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC flags
if(MSVC)
    add_compile_options(/W4 /utf-8)
endif()

# Add FLTK from external directory
add_subdirectory(external/fltk)

# Add Boost headers
add_library(boost INTERFACE)
target_include_directories(boost INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/boost/libs/math/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/boost/libs/config/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/boost/libs/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/boost/libs/assert/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/boost/libs/type_traits/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/boost/libs/static_assert/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/boost/libs/throw_exception/include
)

# Core library
add_library(core STATIC
    src/generators.cpp
    src/stats.cpp
    src/tests.cpp
    src/histogram_widget.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/external/fltk
    ${CMAKE_SOURCE_DIR}/external/fltk
)

target_link_libraries(core PUBLIC
    fltk
    boost
)

# Main program
add_executable(Lab2
    src/main.cpp
)

target_link_libraries(Lab2 PRIVATE core)

if(WIN32)
    set_target_properties(Lab2 PROPERTIES WIN32_EXECUTABLE FALSE)
endif()


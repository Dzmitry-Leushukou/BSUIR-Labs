cmake_minimum_required(VERSION 3.20)

project(Lab5Petri LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8)
endif()

# Reuse the FLTK copy already used by the previous laboratory.
set(FLTK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Lab 1/external/fltk")
if(EXISTS "${FLTK_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("${FLTK_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/fltk")
else()
    find_package(FLTK CONFIG REQUIRED)
endif()

add_executable(Lab5
    src/main.cpp
    src/petri_model.cpp
    src/petri_ui.cpp
)

target_include_directories(Lab5 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${FLTK_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/fltk"
)

target_link_libraries(Lab5 PRIVATE fltk)

if(WIN32)
    set_target_properties(Lab5 PROPERTIES WIN32_EXECUTABLE FALSE)
endif()

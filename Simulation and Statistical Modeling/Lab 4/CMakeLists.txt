cmake_minimum_required(VERSION 3.20)

project(lab4_repair_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /utf-8)
endif()

# Reuse the FLTK copy already kept with the previous laboratory works.
set(FLTK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Lab 2/external/fltk")
add_subdirectory("${FLTK_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/fltk-build" EXCLUDE_FROM_ALL)

add_executable(Lab4
    src/main.cpp
    src/state_graph_widget.cpp
)

target_include_directories(Lab4 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${FLTK_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/fltk-build"
)

target_link_libraries(Lab4 PRIVATE fltk)

if(WIN32)
    set_target_properties(Lab4 PROPERTIES WIN32_EXECUTABLE FALSE)
endif()

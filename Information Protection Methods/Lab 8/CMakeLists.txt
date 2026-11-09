cmake_minimum_required(VERSION 3.20)

project(Lab8 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "Lab8 supports Windows only.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(JPEG REQUIRED)

add_executable(Lab8 src/main.cpp src/JpegSteganography.cpp)

target_link_libraries(Lab8 PRIVATE JPEG::JPEG shell32)

find_file(JPEG_RUNTIME_LIBRARY NAMES libjpeg-62.dll jpeg62.dll
    HINTS "${JPEG_INCLUDE_DIR}/../bin")
if(JPEG_RUNTIME_LIBRARY)
    add_custom_command(TARGET Lab8 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${JPEG_RUNTIME_LIBRARY}" $<TARGET_FILE_DIR:Lab8>)
endif()

if(MSVC)
    target_compile_options(Lab8 PRIVATE /W4 /utf-8)
else()
    target_compile_options(Lab8 PRIVATE -Wall -Wextra -Wpedantic)
endif()

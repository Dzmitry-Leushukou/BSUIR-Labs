cmake_minimum_required(VERSION 4.1)
project(Lab_2)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})

add_executable(Lab_2 main.cpp
        CaesarCipher.cpp
        CaesarCipher.h
        VigenereCipher.cpp
        VigenereCipher.h
        CipherBase.cpp
        CipherBase.h
        ui.cpp
        ui.h)
configure_file(ui.glade ${CMAKE_CURRENT_BINARY_DIR}/ui.glade COPYONLY)
target_include_directories(Lab_2 PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_libraries(Lab_2 PRIVATE ${GTKMM_LIBRARIES} ${GTKMM_LDFLAGS})
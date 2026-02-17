cmake_minimum_required(VERSION 4.1)
project(Lab_3)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})

add_executable(Lab_3 main.cpp
        ui.cpp
        ui.h
        common.cpp
        common.h)
configure_file(ui.glade ${CMAKE_CURRENT_BINARY_DIR}/ui.glade COPYONLY)
target_include_directories(Lab_3 PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_libraries(Lab_3 PRIVATE ${GTKMM_LIBRARIES} ${GTKMM_LDFLAGS})
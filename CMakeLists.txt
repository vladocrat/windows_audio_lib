cmake_minimum_required(VERSION 3.5)

project(sound_capture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(utils)

add_library(sound_capture
    device.h device.cpp
    deviceexplorer.h deviceexplorer.cpp
    general.h
    devicemanager.h devicemanager.cpp)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core)

target_link_libraries(sound_capture Qt6::Core)

include(GNUInstallDirs)
install(TARGETS sound_capture
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

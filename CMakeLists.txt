cmake_minimum_required(VERSION 3.5)

project(microphone_capture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(utils)

add_executable(microphone_capture main.cpp
    recordingdevice.h recordingdevice.cpp
    device.h device.cpp)

target_link_libraries(microphone_capture)

include(GNUInstallDirs)
install(TARGETS microphone_capture
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

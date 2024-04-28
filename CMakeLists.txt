cmake_minimum_required(VERSION 3.5)

project(microphone_capture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(microphone_capture main.cpp)

include(GNUInstallDirs)
install(TARGETS microphone_capture
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

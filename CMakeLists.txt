cmake_minimum_required(VERSION 3.5)

project(sound_capture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(utils)

set(PUBLIC_HEADERS
    include/slk/sound_capture.h
    include/slk/general.h
    include/slk/types.h
    include/slk/audiobuffer.h
    include/slk/audioformat.h
    include/slk/audiofile.h
    include/slk/ringbuffer.h
    include/slk/device.h
    include/slk/inputdevice.h
    include/slk/outputdevice.h
    include/slk/deviceexplorer.h
    include/slk/devicemanager.h
    include/slk/dsp/complex.h
    include/slk/dsp/dsp.h
    include/slk/dsp/filter.h
    include/slk/dsp/noise.h
    include/slk/dsp/window.h
)

set(PRIVATE_HEADERS
    src/deviceinfo.h
)

set(SOURCES
    src/audiobuffer.cpp
    src/audiofile.cpp
    src/audioformat.cpp
    src/complex.cpp
    src/device.cpp
    src/deviceexplorer.cpp
    src/devicemanager.cpp
    src/dsp.cpp
    src/filter.cpp
    src/noise.cpp
    src/ringbuffer.cpp
    src/types.cpp
    src/window.cpp
)

set(PLATFORM_SOURCES "")
set(PLATFORM_PRIVATE_HEADERS "")

if(WIN32)
    list(APPEND PLATFORM_SOURCES
        src/windows/wasapidevice.cpp
        src/windows/wasapiinputdevice.cpp
        src/windows/wasapioutputdevice.cpp
    )
    list(APPEND PLATFORM_PRIVATE_HEADERS
        src/windows/wasapidevice.h
        src/windows/wasapiinputdevice.h
        src/windows/wasapioutputdevice.h
    )
endif()

add_library(sound_capture
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
    ${PLATFORM_PRIVATE_HEADERS}
    ${SOURCES}
    ${PLATFORM_SOURCES}
)

option(BUILD_EXAMPLES OFF)

if (${BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()

option(BUILD_TESTS "Build unit tests" OFF)

if (BUILD_TESTS)
    enable_testing()
    include(cmake/GoogleTest.cmake)
    add_subdirectory(tests)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/slk)

target_include_directories(sound_capture
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        $<$<BOOL:${WIN32}>:${CMAKE_CURRENT_SOURCE_DIR}/src/windows>
)

target_link_libraries(sound_capture
    PUBLIC
        utils_lib
)

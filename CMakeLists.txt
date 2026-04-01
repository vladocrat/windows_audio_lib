cmake_minimum_required(VERSION 3.5)

project(sound_capture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(utils)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core)

add_library(sound_capture
    # Public headers (listed for IDE visibility and AUTOMOC scanning)
    include/slk/sound_capture.h
    include/slk/general.h
    include/slk/types.h
    include/slk/audiobuffer.h
    include/slk/audiodata.h
    include/slk/audioformat.h
    include/slk/audiofile.h
    include/slk/ringbuffer.h
    include/slk/device.h
    include/slk/deviceexplorer.h
    include/slk/devicemanager.h
    include/slk/wasapidevice.h
    include/slk/wasapiinputdevice.h
    include/slk/wasapioutputdevice.h
    include/slk/dsp/complex.h
    include/slk/dsp/dsp.h
    include/slk/dsp/filter.h
    include/slk/dsp/noise.h
    include/slk/dsp/window.h

    # Private headers (AUTOMOC must scan devicethread.h for Q_OBJECT)
    src/devicethread.h
    src/helpers.h

    # Sources
    src/audiobuffer.cpp
    src/audiodata.cpp
    src/audiofile.cpp
    src/audioformat.cpp
    src/complex.cpp
    src/device.cpp
    src/deviceexplorer.cpp
    src/devicemanager.cpp
    src/devicethread.cpp
    src/dsp.cpp
    src/filter.cpp
    src/noise.cpp
    src/ringbuffer.cpp
    src/types.cpp
    src/wasapidevice.cpp
    src/wasapiinputdevice.cpp
    src/wasapioutputdevice.cpp
    src/window.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/slk)

target_include_directories(sound_capture
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(sound_capture
    PUBLIC
        Qt6::Core
        utils_lib
)

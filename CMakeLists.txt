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
    include/slk/dsp/processor.h
    include/slk/dsp/chain.h
    include/slk/dsp/graph.h
    include/slk/dsp/hertz.h
    include/slk/dsp/db.h
    include/slk/dsp/kahn.h
)

set(PRIVATE_HEADERS
    src/deviceinfo.h
)

set(SOURCES
    src/audiobuffer.cpp
    src/audiofile.cpp
    src/audioformat.cpp
    src/complex.cpp
    src/db.cpp
    src/device.cpp
    src/deviceexplorer.cpp
    src/devicemanager.cpp
    src/dsp.cpp
    src/filter.cpp
    src/kahn.cpp
    src/noise.cpp
    src/ringbuffer.cpp
    src/types.cpp
    src/window.cpp
)

set(PLATFORM_SOURCES "")
set(PLATFORM_PRIVATE_HEADERS "")

if(WIN32)
    list(APPEND PLATFORM_SOURCES
        src/windows/deviceexplorer.cpp
        src/windows/wasapidevice.cpp
        src/windows/wasapiinputdevice.cpp
        src/windows/wasapioutputdevice.cpp
    )
    list(APPEND PLATFORM_PRIVATE_HEADERS
        src/windows/deviceexplorer.h
        src/windows/wasapidevice.h
        src/windows/wasapiinputdevice.h
        src/windows/wasapioutputdevice.h
    )
elseif(APPLE)
    list(APPEND PLATFORM_SOURCES
        src/macos/deviceexplorer.cpp
        src/macos/coreaudiodevice.cpp
        src/macos/coreaudioinputdevice.cpp
        src/macos/coreaudiooutputdevice.cpp
    )
    list(APPEND PLATFORM_PRIVATE_HEADERS
        src/macos/deviceexplorer.h
        src/macos/coreaudiodevice.h
        src/macos/coreaudioinputdevice.h
        src/macos/coreaudiooutputdevice.h
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
        $<$<BOOL:${APPLE}>:${CMAKE_CURRENT_SOURCE_DIR}/src/macos>
)

if(APPLE)
    find_library(CORE_AUDIO_FRAMEWORK CoreAudio REQUIRED)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
endif()

target_link_libraries(sound_capture
    PUBLIC
        utils_lib
    PRIVATE
        $<$<BOOL:${APPLE}>:${CORE_AUDIO_FRAMEWORK}>
        $<$<BOOL:${APPLE}>:${CORE_FOUNDATION_FRAMEWORK}>
        # WASAPI uses KS audio format GUIDs (KSDATAFORMAT_SUBTYPE_*). MSVC
        # auto-resolves them via #pragma comment(lib) in the SDK headers;
        # MinGW doesn't, so the link must request ksuser explicitly. No-op
        # on MSVC.
        $<$<BOOL:${WIN32}>:ksuser>
)

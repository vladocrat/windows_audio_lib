# ── Unit tests ───────────────────────────────────────────────────────────────
# No real audio hardware required. Build + run on every CI platform.
file(GLOB UNIT_TEST_SOURCES CONFIGURE_DEPENDS unit/*.cpp)

add_executable(audio_lib_unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(audio_lib_unit_tests PRIVATE sound_capture GTest::gtest_main)
target_compile_definitions(audio_lib_unit_tests PRIVATE
    AUDIO_LIB_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

include(GoogleTest)
gtest_discover_tests(audio_lib_unit_tests)

# ── Integration tests ────────────────────────────────────────────────────────
# Need real WASAPI hardware (or a working CoreAudio/ALSA stub on
# macOS/Linux). Built on every platform so changes don't bit-rot, but only
# *executed* on dev machines with hardware. CI's ctest invocation should
# target audio_lib_unit_tests by name and skip these.
file(GLOB INTEGRATION_TEST_SOURCES CONFIGURE_DEPENDS integration/*.cpp)

add_executable(audio_lib_integration_tests ${INTEGRATION_TEST_SOURCES})
target_link_libraries(audio_lib_integration_tests PRIVATE sound_capture GTest::gtest_main)

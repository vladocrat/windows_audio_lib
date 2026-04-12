add_executable(audio_lib_tests
    test_audiobuffer.cpp
    test_ringbuffer.cpp
    test_dsp.cpp
    test_literals.cpp
    test_audioformat.cpp
    test_devices.cpp
)

target_link_libraries(audio_lib_tests PRIVATE sound_capture GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(audio_lib_tests)

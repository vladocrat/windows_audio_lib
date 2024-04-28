add_library(utils_lib STATIC utils.h)

set_target_properties(utils_lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(utils_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

project(pludux-lib LANGUAGES CXX)

add_subdirectory(sources)

if(NOT EMSCRIPTEN AND BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_subdirectory(lib)
add_subdirectory(cli)
add_subdirectory(gui)

if(NOT EMSCRIPTEN AND BUILD_TESTING)
    add_subdirectory(tests)
endif()


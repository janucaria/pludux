project(pludux LANGUAGES CXX)

add_subdirectory(sources)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

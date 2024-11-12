project(pludux-lib LANGUAGES CXX)

find_package(GTest REQUIRED)

add_subdirectory(sources)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

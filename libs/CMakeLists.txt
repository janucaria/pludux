project(pludux-lib LANGUAGES CXX)

find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)

add_subdirectory(sources)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

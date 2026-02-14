cmake_minimum_required(VERSION 3.30)
project(pludux
  VERSION 0.1.0
  HOMEPAGE_URL "https://github.com/janucaria/pludux"
  LANGUAGES CXX
)

include(CTest)
include(FetchContent)

if(NOT EMSCRIPTEN)
  set(BUILD_GMOCK  OFF)
  set(INSTALL_GTEST OFF)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    OVERRIDE_FIND_PACKAGE
  )

  find_package(googletest REQUIRED)
endif()

add_subdirectory(libs)
add_subdirectory(apps)

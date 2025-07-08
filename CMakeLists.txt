cmake_minimum_required(VERSION 3.30)
project(pludux
  VERSION 0.0.6
  HOMEPAGE_URL "https://github.com/janucaria/pludux"
  LANGUAGES CXX
)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)

include(CTest)
include(FetchContent)

add_subdirectory(libs)
add_subdirectory(apps)

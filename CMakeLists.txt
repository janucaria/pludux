cmake_minimum_required(VERSION 3.23)

project(pludux LANGUAGES CXX)

include(CTest)

find_package(GTest REQUIRED)

add_subdirectory(libs)

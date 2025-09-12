project(pludux-backtest-lib LANGUAGES CXX)

FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY https://github.com/d99kris/rapidcsv.git
  GIT_TAG        v8.88
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
  ctre
  GIT_REPOSITORY https://github.com/hanickadot/compile-time-regular-expressions.git
  GIT_TAG        v3.10.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
  PATCH_COMMAND 
    ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/patch/ctre/
      <SOURCE_DIR>/
)

FetchContent_Declare(
  jsoncons
  GIT_REPOSITORY https://github.com/danielaparker/jsoncons.git
  GIT_TAG        v1.4.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
)

find_package(rapidcsv REQUIRED)
find_package(ctre REQUIRED)
find_package(jsoncons REQUIRED)


add_library(${PROJECT_NAME})
add_library(pludux::backtest-lib ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC
    sources/backtest.cpp

    sources/backtest/trade_entry.cpp
    sources/backtest/trade_exit.cpp
    sources/backtest/trade_position.cpp
    sources/backtest/trade_record.cpp
    sources/backtest/trade_session.cpp

    sources/backtest/backtest_summary.cpp
    sources/backtest/asset.cpp
    sources/backtest/strategy.cpp
    sources/backtest/profile.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    pludux::pludux-lib
    nlohmann_json::nlohmann_json
    rapidcsv
    ctre::ctre
    jsoncons
)

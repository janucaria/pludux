project(pludux-backtest-lib)

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

find_package(rapidcsv REQUIRED)
find_package(ctre REQUIRED)


add_library(${PROJECT_NAME})
add_library(pludux::backtest-lib ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC

    sources/backtest/trade_entry.cpp
    sources/backtest/trade_exit.cpp
    sources/backtest/trade_position.cpp
    sources/backtest/trade_record.cpp
)

target_sources(${PROJECT_NAME}
  PUBLIC
  FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/sources
    FILES
    
    sources/backtest/trade_session.cxx
    
    sources/backtest/asset.cxx
    sources/backtest/profile.cxx
    sources/backtest/backtest_summary.cxx
    sources/backtest/strategy.cxx

    sources/backtest.cxx
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
)

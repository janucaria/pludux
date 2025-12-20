project(pludux-backtest-lib)

FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY https://github.com/d99kris/rapidcsv.git
  GIT_TAG        v8.90
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

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    CXX_SCAN_FOR_MODULES OFF
)

target_sources(${PROJECT_NAME}
  PUBLIC
  FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/sources
    FILES
    
    sources/backtest/trade_entry.cxx
    sources/backtest/trade_exit.cxx
    sources/backtest/trade_record.cxx
    sources/backtest/trade_position.cxx
    sources/backtest/trade_session.cxx
    
    sources/backtest/asset.cxx
    sources/backtest/strategy.cxx
    sources/backtest/market.cxx
    sources/backtest/broker.cxx
    sources/backtest/profile.cxx
    sources/backtest/backtest_summary.cxx

    sources/backtest.cxx
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    pludux::pludux-lib
    rapidcsv
    ctre::ctre
)

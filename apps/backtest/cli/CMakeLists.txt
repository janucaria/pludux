project(pludux-backtest-cli LANGUAGES CXX)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PRIVATE
    sources/main.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    pludux::backtest-lib
)

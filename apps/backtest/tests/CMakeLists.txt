set(PLUDUX_TEST_SOURCES
  src/test_backtest_setup.cpp
  src/test_backtest_runner.cpp
  src/test_portfolio.cpp
  src/test_portfolio_runner.cpp
  src/test_watchlist.cpp
  src/test_config_parser.cpp
  src/test_erased_plot_method.cpp
  src/test_strategy_inputs.cpp
  src/test_strategy_performance.cpp
  src/test_strategy_session.cpp
  src/test_stop_target_price.cpp
  src/test_execution_filter.cpp
  src/test_entry_order_sizing.cpp
  src/test_position_sizing.cpp
  src/test_pyramiding_layer.cpp
  src/test_trade_session.cpp
)

foreach(PLUDUX_TEST_SOURCE ${PLUDUX_TEST_SOURCES})
  get_filename_component(PLUDUX_TEST_NAME ${PLUDUX_TEST_SOURCE} NAME_WE)

  set(PLUDUX_TEST_NAME "pludux-backtest-${PLUDUX_TEST_NAME}")

  add_executable(${PLUDUX_TEST_NAME} ${PLUDUX_TEST_SOURCE})
  target_link_libraries(${PLUDUX_TEST_NAME}
    PRIVATE
      GTest::gtest_main
      pludux::backtest-lib
  )
  target_compile_definitions(${PLUDUX_TEST_NAME}
    PRIVATE
      PLUDUX_BACKTEST_SAMPLE_DIR="${CMAKE_SOURCE_DIR}/data/backtest/samples"
  )
  if(MSVC AND PLUDUX_TEST_NAME STREQUAL
     "pludux-backtest-test_strategy_inputs")
    target_compile_options(${PLUDUX_TEST_NAME} PRIVATE /bigobj)
  endif()
  add_test(NAME ${PLUDUX_TEST_NAME} COMMAND ${PLUDUX_TEST_NAME})
endforeach()

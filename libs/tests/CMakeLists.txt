set(PLUDUX_TEST_SOURCES
  src/test_asset_history.cpp
  src/test_config_parser.cpp
  src/test_series.cpp
  src/test_ta.cpp

  src/test_bullish_divergence_series.cpp

  src/test_abs_diff_method.cpp
  src/test_arithmetic_method.cpp
  src/test_atr_method.cpp
  src/test_data_method.cpp
  src/test_ema_method.cpp
  src/test_hma_method.cpp
  src/test_rma_method.cpp
  src/test_sma_method.cpp
  src/test_value_method.cpp
  src/test_wma_method.cpp

  src/test_all_of_filter.cpp
  src/test_any_of_filter.cpp
  src/test_crossover_filter.cpp
  src/test_crossunder_filter.cpp
  src/test_equal_filter.cpp
  src/test_greater_equal_filter.cpp
  src/test_greater_than_filter.cpp
  src/test_less_equal_filter.cpp
  src/test_less_than_filter.cpp
  src/test_not_equal_filter.cpp

)

foreach(PLUDUX_TEST_SOURCE ${PLUDUX_TEST_SOURCES})
  get_filename_component(PLUDUX_TEST_NAME ${PLUDUX_TEST_SOURCE} NAME_WE)
  add_executable(${PLUDUX_TEST_NAME} ${PLUDUX_TEST_SOURCE})
  target_link_libraries(${PLUDUX_TEST_NAME}
    PRIVATE
      GTest::gtest_main
      ${CMAKE_PROJECT_NAME}::${PROJECT_NAME}
  )
  add_test(NAME ${PLUDUX_TEST_NAME} COMMAND ${PLUDUX_TEST_NAME})
endforeach()

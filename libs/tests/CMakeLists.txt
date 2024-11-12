set(PLUDUX_TEST_SOURCES
  src/test_quote.cpp
  src/test_asset.cpp
  src/test_series.cpp
  src/test_ta.cpp

  src/test_field_method.cpp
  src/test_value_method.cpp
  src/test_sma_method.cpp

  src/test_all_of_filter.cpp
  src/test_any_of_filter.cpp
  src/test_greater_than_filter.cpp
  src/test_less_than_filter.cpp
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

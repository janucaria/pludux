set(PLUDUX_TEST_SOURCES
  src/test_quote.cpp
  src/test_asset.cpp
  src/test_ta.cpp
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

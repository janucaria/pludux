SET(JSONCONS_BUILD_TESTS OFF)
FetchContent_Declare(
  jsoncons
  GIT_REPOSITORY https://github.com/danielaparker/jsoncons.git
  GIT_TAG        v1.4.1
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)

find_package(jsoncons REQUIRED)

add_library(${PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    CXX_SCAN_FOR_MODULES OFF
)

target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS src/
      FILES

        src/series/series_output.cxx

        src/series/ref_series.cxx
        src/series/output_by_name_series.cxx
        src/series/lookback_series.cxx
        src/series/data_series.cxx
        src/series/repeat_series.cxx
        src/series/operators_series.cxx
        src/series/change_series.cxx
        src/series/roc_series.cxx

        src/series/poly_series.cxx
        
        src/series/sma_series.cxx
        src/series/wma_series.cxx
        src/series/ema_series.cxx
        src/series/hma_series.cxx
        src/series/rma_series.cxx
        src/series/macd_series.cxx

        src/series/rvol_series.cxx
        src/series/kc_series.cxx
        src/series/bb_series.cxx

        src/series/tr_series.cxx
        src/series/atr_series.cxx

        src/series/stoch_series.cxx
        src/series/rsi_series.cxx
        src/series/stoch_rsi_series.cxx

        src/series/pivot_lows_series.cxx
        src/series/bullish_divergence_series.cxx
        src/series/hidden_bullish_divergence_series.cxx

        src/series.cxx


        src/asset_history.cxx
        src/asset_snapshot.cxx
        src/config_parser.cxx

        
        src/screener/methods/screener_method.cxx

        src/screener/methods/method_registry.cxx
        src/screener/methods/reference_method.cxx

        src/screener/methods/lookback_method.cxx
        src/screener/methods/output_by_name_method.cxx
        src/screener/methods/value_method.cxx
        src/screener/methods/ohlcv_method.cxx
        src/screener/methods/data_method.cxx
        src/screener/methods/arithmetic_method.cxx
        src/screener/methods/ta_with_period_method.cxx
        src/screener/methods/atr_method.cxx
        src/screener/methods/bb_method.cxx
        src/screener/methods/changes_method.cxx
        src/screener/methods/kc_method.cxx
        src/screener/methods/macd_method.cxx
        src/screener/methods/stoch_method.cxx
        src/screener/methods/stoch_rsi_method.cxx
        src/screener/methods/bullish_divergence_method.cxx
        src/screener/methods/hidden_bullish_divergence_method.cxx
        
        src/screener/filters/screener_filter.cxx
        src/screener/filters/all_of_filter.cxx
        src/screener/filters/any_of_filter.cxx
        src/screener/filters/boolean_filter.cxx
        src/screener/filters/comparison_filter.cxx
        src/screener/filters/logical_filter.cxx
        src/screener/filters/crossover_filter.cxx
        src/screener/filters/crossunder_filter.cxx

        src/screener.cxx

        src/module.cxx
)

target_compile_features(${PROJECT_NAME}
  PUBLIC
    cxx_std_23
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    jsoncons
)

if(EMSCRIPTEN)
  target_compile_options(${PROJECT_NAME} PUBLIC
    "-fwasm-exceptions"
  )
  
  target_link_options(${PROJECT_NAME} PUBLIC
    "-fwasm-exceptions"
  )
endif()

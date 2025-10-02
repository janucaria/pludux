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

        src/asset_data.cxx
        src/asset_series.cxx
        src/asset_history.cxx
        src/asset_snapshot.cxx
        
        src/series/method_output.cxx
        src/series/method_call_context.cxx

        src/series/any_method_context.cxx
        src/series/any_method.cxx
        src/series/method_registry.cxx
        src/series/default_method_context.cxx

        src/series/value_method.cxx
        src/series/data_method.cxx
        src/series/ohlcv_method.cxx
        src/series/change_method.cxx
        src/series/lookback_method.cxx
        src/series/select_output_method.cxx
        src/series/reference_method.cxx

        src/series/operators_method.cxx
        src/series/highest_method.cxx
        src/series/lowest_method.cxx

        src/series/sma_method.cxx
        src/series/ema_method.cxx
        src/series/rma_method.cxx
        src/series/wma_method.cxx
        src/series/hma_method.cxx
        src/series/macd_method.cxx
        
        src/series/tr_method.cxx
        src/series/atr_method.cxx

        src/series/roc_method.cxx
        src/series/rsi_method.cxx
        src/series/rvol_method.cxx

        src/series/bb_method.cxx
        src/series/kc_method.cxx

        src/series/stoch_method.cxx
        src/series/stoch_rsi_method.cxx

        src/series.cxx

        # src/series/bullish_divergence_method.cxx
        # src/series/hidden_bullish_divergence_method.cxx
        
        src/screener/filters/screener_filter.cxx
        src/screener/filters/all_of_filter.cxx
        src/screener/filters/any_of_filter.cxx
        src/screener/filters/boolean_filter.cxx
        src/screener/filters/comparison_filter.cxx
        src/screener/filters/logical_filter.cxx
        src/screener/filters/crossover_filter.cxx
        src/screener/filters/crossunder_filter.cxx

        src/screener.cxx
        
        src/config_parser.cxx

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

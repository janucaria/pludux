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
        
        src/screener/methods/method_output.cxx
        src/screener/methods/method_call_context.cxx

        src/screener/methods/any_method_context.cxx
        src/screener/methods/any_method.cxx
        src/screener/methods/method_registry.cxx
        src/screener/methods/default_method_context.cxx

        src/screener/methods/value_method.cxx
        src/screener/methods/data_method.cxx
        src/screener/methods/ohlcv_method.cxx
        src/screener/methods/change_method.cxx
        src/screener/methods/lookback_method.cxx
        src/screener/methods/select_output_method.cxx
        src/screener/methods/reference_method.cxx

        src/screener/methods/operators_method.cxx
        src/screener/methods/highest_method.cxx
        src/screener/methods/lowest_method.cxx

        src/screener/methods/sma_method.cxx
        src/screener/methods/ema_method.cxx
        src/screener/methods/rma_method.cxx
        src/screener/methods/wma_method.cxx
        src/screener/methods/hma_method.cxx
        src/screener/methods/macd_method.cxx
        
        src/screener/methods/tr_method.cxx
        src/screener/methods/atr_method.cxx

        src/screener/methods/roc_method.cxx
        src/screener/methods/rsi_method.cxx
        src/screener/methods/rvol_method.cxx

        src/screener/methods/bb_method.cxx
        src/screener/methods/kc_method.cxx

        src/screener/methods/stoch_method.cxx
        src/screener/methods/stoch_rsi_method.cxx

        # src/screener/methods/bullish_divergence_method.cxx
        # src/screener/methods/hidden_bullish_divergence_method.cxx
        
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

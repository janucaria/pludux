FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.12.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
)

find_package(nlohmann_json REQUIRED)

add_library(${PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})


target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET CXX_MODULES
      FILES
        include/pludux/series/atr_series.cxx
        include/pludux/series/bb_series.cxx
        include/pludux/series/binary_fn_series.cxx
        include/pludux/series/bullish_divergence_series.cxx
        include/pludux/series/change_series.cxx
        include/pludux/series/data_series.cxx
        include/pludux/series/ema_series.cxx
        include/pludux/series/hidden_bullish_divergence_series.cxx
        include/pludux/series/hma_series.cxx
        include/pludux/series/kc_series.cxx
        include/pludux/series/macd_series.cxx
        include/pludux/series/pivot_lows_series.cxx
        include/pludux/series/poly_series.cxx
        include/pludux/series/ref_series.cxx
        include/pludux/series/repeat_series.cxx
        include/pludux/series/rma_series.cxx
        include/pludux/series/roc_series.cxx
        include/pludux/series/rsi_series.cxx
        include/pludux/series/rvol_series.cxx
        include/pludux/series/sma_series.cxx
        include/pludux/series/stoch_rsi_series.cxx
        include/pludux/series/stoch_series.cxx
        include/pludux/series/sub_series.cxx
        include/pludux/series/tr_series.cxx
        include/pludux/series/unary_fn_series.cxx
        include/pludux/series/wma_series.cxx
        include/pludux/series.cxx

               
        include/pludux/asset_history.cxx
        include/pludux/asset_snapshot.cxx
        include/pludux/config_parser.cxx
        include/pludux/ta.cxx

        
        include/pludux/screener.cxx
        include/pludux/screener/arithmetic_method.cxx
        include/pludux/screener/atr_method.cxx
        include/pludux/screener/bb_method.cxx
        include/pludux/screener/bullish_divergence_method.cxx
        include/pludux/screener/changes_method.cxx
        include/pludux/screener/data_method.cxx
        include/pludux/screener/hidden_bullish_divergence_method.cxx
        include/pludux/screener/kc_method.cxx
        include/pludux/screener/macd_method.cxx
        include/pludux/screener/ohlcv_method.cxx
        include/pludux/screener/screener_method.cxx
        include/pludux/screener/stoch_method.cxx
        include/pludux/screener/stoch_rsi_method.cxx
        include/pludux/screener/ta_with_period_method.cxx
        include/pludux/screener/value_method.cxx

        include/pludux/screener/all_of_filter.cxx
        include/pludux/screener/any_of_filter.cxx
        include/pludux/screener/boolean_filter.cxx
        include/pludux/screener/comparison_filter.cxx
        include/pludux/screener/crossover_filter.cxx
        include/pludux/screener/crossunder_filter.cxx
        include/pludux/screener/logical_filter.cxx
        include/pludux/screener/screener_filter.cxx

)

target_compile_features(${PROJECT_NAME}
  PUBLIC
    cxx_std_23
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    nlohmann_json::nlohmann_json
)

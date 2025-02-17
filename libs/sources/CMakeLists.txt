FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
)

find_package(nlohmann_json REQUIRED)

add_library(${PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC
    src/asset_history.cpp
    src/asset_snapshot.cpp
    src/config_parser.cpp

    src/screener/abs_diff_method.cpp
    src/screener/atr_method.cpp
    src/screener/bb_method.cpp
    src/screener/bullish_divergence_method.cpp
    src/screener/changes_method.cpp
    src/screener/data_method.cpp
    src/screener/hidden_bullish_divergence_method.cpp
    src/screener/kc_method.cpp
    src/screener/macd_method.cpp
    src/screener/stoch_method.cpp
    src/screener/stoch_rsi_method.cpp
    src/screener/value_method.cpp

    src/screener/all_of_filter.cpp
    src/screener/any_of_filter.cpp
    src/screener/crossover_filter.cpp
    src/screener/crossunder_filter.cpp

  INTERFACE
    FILE_SET HEADERS
      BASE_DIRS include/
      FILES
        include/pludux/asset_history.cpp
        include/pludux/asset_snapshot.hpp
        include/pludux/config_parser.hpp
        include/pludux/screener.hpp
        include/pludux/series.hpp
        include/pludux/ta.hpp

        include/pludux/screener/abs_diff_method.hpp 
        include/pludux/screener/arithmetic_method.hpp 
        include/pludux/screener/atr_method.hpp 
        include/pludux/screener/bb_method.hpp 
        include/pludux/screener/bullish_divergence_method.hpp
        include/pludux/screener/changes_method.hpp 
        include/pludux/screener/data_method.hpp 
        include/pludux/screener/hidden_bullish_divergence_method.hpp
        include/pludux/screener/kc_method.hpp
        include/pludux/screener/macd_method.hpp
        include/pludux/screener/screener_method.hpp 
        include/pludux/screener/stoch_method.hpp 
        include/pludux/screener/stoch_rsi_method.hpp 
        include/pludux/screener/ta_with_period_method.hpp 
        include/pludux/screener/value_method.hpp 
        
        include/pludux/screener/all_of_filter.hpp
        include/pludux/screener/any_of_filter.hpp
        include/pludux/screener/comparison_filter.hpp
        include/pludux/screener/crossover_filter.hpp
        include/pludux/screener/crossunder_filter.hpp
        include/pludux/screener/screener_filter.hpp 

        include/pludux/series/atr_series.hpp
        include/pludux/series/bb_series.hpp
        include/pludux/series/binary_fn_series.hpp
        include/pludux/series/bullish_divergence_series.hpp
        include/pludux/series/change_series.hpp
        include/pludux/series/data_series.hpp
        include/pludux/series/ema_series.hpp
        include/pludux/series/hidden_bullish_divergence_series.hpp
        include/pludux/series/hma_series.hpp
        include/pludux/series/kc_series.hpp
        include/pludux/series/macd_series.hpp
        include/pludux/series/pivot_lows_series.hpp
        include/pludux/series/poly_series.hpp
        include/pludux/series/ref_series.hpp
        include/pludux/series/repeat_series.hpp
        include/pludux/series/rma_series.hpp
        include/pludux/series/roc_series.hpp
        include/pludux/series/rsi_series.hpp
        include/pludux/series/rvol_series.hpp
        include/pludux/series/sma_series.hpp
        include/pludux/series/stoch_rsi_series.hpp
        include/pludux/series/stoch_series.hpp
        include/pludux/series/sub_series.hpp
        include/pludux/series/tr_series.hpp
        include/pludux/series/unary_fn_series.hpp
        include/pludux/series/wma_series.hpp
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

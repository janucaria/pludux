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

    src/screener/abs_diff_method.cpp
    src/screener/arithmetic_method.cpp
    src/screener/atr_method.cpp
    src/screener/bb_method.cpp
    src/screener/bullish_divergence_method.cpp
    src/screener/changes_method.cpp
    src/screener/data_method.cpp
    src/screener/hidden_bullish_divergence_method.cpp
    src/screener/kc_method.cpp
    src/screener/lookback_method.cpp
    src/screener/macd_method.cpp
    src/screener/output_by_name_method.cpp
    src/screener/screener_method.cpp
    src/screener/stoch_method.cpp
    src/screener/stoch_rsi_method.cpp
    src/screener/reference_method.cpp
    src/screener/value_method.cpp

    src/screener/method_registry.cpp

    src/screener/all_of_filter.cpp
    src/screener/any_of_filter.cpp
    src/screener/crossover_filter.cpp
    src/screener/crossunder_filter.cpp
    src/screener/screener_filter.cpp

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
        src/series/stoch_series.cxx

        src/series/poly_series.cxx
        
        src/series/sma_series.cxx
        src/series/wma_series.cxx
        src/series/ema_series.cxx
        src/series/hma_series.cxx
        src/series/rma_series.cxx

        src/series/rvol_series.cxx
        src/series/kc_series.cxx
        src/series/bb_series.cxx

        src/series/tr_series.cxx
        src/series/atr_series.cxx

        src/series/pivot_lows_series.cxx
        src/series/bullish_divergence_series.cxx
        src/series/hidden_bullish_divergence_series.cxx

        src/series.cxx


        src/asset_history.cxx
        src/asset_snapshot.cxx
        src/config_parser.cxx
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

if(EMSCRIPTEN)
  target_compile_options(${PROJECT_NAME} PUBLIC
    "-fwasm-exceptions"
  )
  
  target_link_options(${PROJECT_NAME} PUBLIC
    "-fwasm-exceptions"
  )
endif()

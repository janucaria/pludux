SET(JSONCONS_BUILD_TESTS OFF)
FetchContent_Declare(
  jsoncons
  GIT_REPOSITORY https://github.com/danielaparker/jsoncons.git
  GIT_TAG        v1.6.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)

FetchContent_Declare(
  boost_mp11
  GIT_REPOSITORY https://github.com/boostorg/mp11.git
  GIT_TAG        boost-1.91.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
  PATCH_COMMAND
    ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/../patch/mp11/
      <SOURCE_DIR>/
)

find_package(jsoncons REQUIRED)
find_package(boost_mp11 REQUIRED)

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

        src/myers_diff.cxx

        src/asset_data.cxx
        src/asset_series.cxx
        src/asset_history.cxx
        src/asset_quote_field_resolver.cxx
        src/asset_snapshot.cxx
        src/methods/value_method.cxx
        src/methods/data_method.cxx
        src/methods/ohlcv_method.cxx
        src/methods/change_method.cxx
        src/methods/lookback_method.cxx
        src/methods/select_output_method.cxx
        src/methods/series_node_method.cxx
        src/methods/series_value_method.cxx

        src/methods/operators_method.cxx
        src/methods/highest_method.cxx
        src/methods/lowest_method.cxx
        src/methods/percentage_method.cxx
        src/methods/stddev_method.cxx

        src/methods/sma_method.cxx
        src/methods/ema_method.cxx
        src/methods/rma_method.cxx
        src/methods/wma_method.cxx
        src/methods/hma_method.cxx
        src/methods/adaptive_ma_method.cxx
        src/methods/macd_method.cxx
        
        src/methods/tr_method.cxx
        src/methods/atr_method.cxx

        src/methods/roc_method.cxx
        src/methods/rsi_method.cxx
        src/methods/rvol_method.cxx

        src/methods/bb_method.cxx
        src/methods/kc_method.cxx
        src/methods/donchian_channel_method.cxx

        src/methods/stoch_method.cxx
        src/methods/stoch_rsi_method.cxx
        
        src/methods/all_of_method.cxx
        src/methods/any_of_method.cxx
        src/methods/comparison_method.cxx
        src/methods/boolean_method.cxx
        src/methods/logical_method.cxx
        src/methods/crossover_method.cxx
        src/methods/crossunder_method.cxx
        
        src/method_key.cxx
        src/ordered_named_registry.cxx
        src/method_contextable.cxx
        src/evaluate_series_method.cxx
        src/hash_series_method.cxx
        src/any_method_context.cxx

        src/methods/any_series_method.cxx

        src/methods.cxx

        src/ma_node_type.cxx
        
        src/nodes/numeric_input_node.cxx
        src/nodes/value_node.cxx
        src/nodes/data_node.cxx
        src/nodes/ohlcv_node.cxx
        src/nodes/erased_node.cxx
        src/nodes/change_node.cxx
        src/nodes/lookback_node.cxx
        src/nodes/select_output_node.cxx
        src/nodes/series_node.cxx
        src/nodes/series_value_node.cxx

        src/nodes/operators_node.cxx
        src/nodes/highest_node.cxx
        src/nodes/lowest_node.cxx
        src/nodes/percentage_node.cxx
        src/nodes/stddev_node.cxx

        src/nodes/sma_node.cxx
        src/nodes/ema_node.cxx
        src/nodes/rma_node.cxx
        src/nodes/wma_node.cxx
        src/nodes/hma_node.cxx
        src/nodes/macd_node.cxx
        
        src/nodes/tr_node.cxx
        src/nodes/atr_node.cxx

        src/nodes/roc_node.cxx
        src/nodes/rsi_node.cxx
        src/nodes/rvol_node.cxx

        src/nodes/bb_node.cxx
        src/nodes/kc_node.cxx
        src/nodes/donchian_channel_node.cxx

        src/nodes/stoch_node.cxx
        src/nodes/stoch_rsi_node.cxx
        
        src/nodes/all_of_node.cxx
        src/nodes/any_of_node.cxx
        src/nodes/comparison_node.cxx
        src/nodes/boolean_node.cxx
        src/nodes/logical_node.cxx
        src/nodes/crossover_node.cxx
        src/nodes/crossunder_node.cxx

        src/nodes.cxx

        src/node_to_erased_method.cxx

        src/series_evaluation_results.cxx
        src/default_method_context.cxx
        src/config_parser.cxx

        src/pludux.cxx
)

target_compile_features(${PROJECT_NAME}
  PUBLIC
    cxx_std_23
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    jsoncons
    Boost::mp11
)

if(EMSCRIPTEN)
  target_compile_options(${PROJECT_NAME} PUBLIC
    "-fwasm-exceptions"
  )
  
  target_link_options(${PROJECT_NAME} PUBLIC
    "-fwasm-exceptions"
  )
endif()

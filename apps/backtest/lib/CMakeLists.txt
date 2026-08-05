project(pludux-backtest-lib)

SET(JSONCONS_BUILD_TESTS OFF)
FetchContent_Declare(
  jsoncons
  GIT_REPOSITORY https://github.com/danielaparker/jsoncons.git
  GIT_TAG        v1.8.1
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)

FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY https://github.com/d99kris/rapidcsv.git
  GIT_TAG        v8.99
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
  ctre
  GIT_REPOSITORY https://github.com/hanickadot/compile-time-regular-expressions.git
  GIT_TAG        v3.11.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  OVERRIDE_FIND_PACKAGE
  PATCH_COMMAND 
    ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/patch/ctre/
      <SOURCE_DIR>/
)

FetchContent_Declare(
  boost_math
  GIT_REPOSITORY https://github.com/boostorg/math.git
  GIT_TAG        boost-1.91.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  EXCLUDE_FROM_ALL
  SOURCE_SUBDIR  _pludux_header_only
)

find_package(jsoncons REQUIRED)
find_package(rapidcsv REQUIRED)
find_package(ctre REQUIRED)
FetchContent_MakeAvailable(boost_math)


add_library(${PROJECT_NAME})
add_library(pludux::backtest-lib ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    CXX_SCAN_FOR_MODULES OFF
)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif()

target_sources(${PROJECT_NAME}
  PUBLIC
  FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/sources
    FILES

    sources/backtest/store_handle.cxx
    sources/backtest/store_slot.cxx
    sources/backtest/store_data_resolver.cxx

    sources/backtest/method_serializer_mixin.cxx

    sources/backtest/execution_model.cxx
    sources/backtest/intrabar_path.cxx

    sources/backtest/plots/plot_method_contextable.cxx
    sources/backtest/plots/erased_plot_method_context.cxx
    
    sources/backtest/plots/source/erased_plot_source_method.cxx
    sources/backtest/plots/source/constant_plot_source_method.cxx
    sources/backtest/plots/source/series_plot_source_method.cxx

    sources/backtest/plots/erased_plot_method.cxx
    sources/backtest/plots/hline_plot_method.cxx
    sources/backtest/plots/line_plot_method.cxx
    sources/backtest/plots/histogram_plot_method.cxx
    sources/backtest/plots/momentum_histogram_plot_method.cxx

    sources/backtest/plots.cxx
    sources/backtest/plot_group.cxx
    sources/backtest/plot_method_parser.cxx
    sources/backtest/plot_source_method_parser.cxx
    
    sources/backtest/trade_entry.cxx
    sources/backtest/trade_exit.cxx
    sources/backtest/take_profit_level.cxx
    sources/backtest/signal_exit_state.cxx
    sources/backtest/stop_loss_level.cxx
    sources/backtest/trade_event.cxx
    sources/backtest/open_position_snapshot.cxx
    sources/backtest/closed_trade.cxx
    sources/backtest/trade_position.cxx
    sources/backtest/trade_session.cxx

    sources/backtest/strategy_intent.cxx
    sources/backtest/strategy_closed_position.cxx
    sources/backtest/strategy_session.cxx
    sources/backtest/strategy_performance_model.cxx
    sources/backtest/strategy_performance.cxx
    sources/backtest/execution_filter_method_context.cxx
    sources/backtest/strategy_performance_method.cxx
    sources/backtest/strategy_performance_node.cxx
    
    sources/backtest/asset.cxx
    sources/backtest/strategy.cxx
    sources/backtest/strategy_inputs.cxx
    sources/backtest/market.cxx
    sources/backtest/broker.cxx
    sources/backtest/position_sizing.cxx
    sources/backtest/entry_order_sizing.cxx
    sources/backtest/profile.cxx
    sources/backtest/backtest_timeline.cxx
    sources/backtest/backtest_method_context.cxx
    sources/backtest/pyramiding_layer_method.cxx
    sources/backtest/pyramiding_layer_node.cxx
    sources/backtest/drawdown_method.cxx
    sources/backtest/drawdown_node.cxx
    sources/backtest/equity_method.cxx
    sources/backtest/equity_node.cxx
    sources/backtest/risk_distance_method.cxx
    sources/backtest/risk_distance_node.cxx
    sources/backtest/position_method.cxx
    sources/backtest/position_node.cxx
    sources/backtest/stop_target_price_method.cxx
    sources/backtest/stop_target_price_node.cxx
    sources/backtest/backtest.cxx
    sources/backtest/portfolio.cxx
    sources/backtest/portfolio_timeline.cxx
    sources/backtest/portfolio_results.cxx
    
    sources/backtest/store_descriptor.cxx
    sources/backtest/store_arena.cxx
    sources/backtest/store.cxx

    sources/backtest/backtest_runner.cxx
    sources/backtest/portfolio_runner.cxx

    sources/backtest/config_parser.cxx
    sources/backtest/strategy_parser.cxx

    sources/backtest.cxx
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    pludux::pludux-lib
    jsoncons
    rapidcsv
    ctre::ctre
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${boost_math_SOURCE_DIR}/include
)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    BOOST_MATH_STANDALONE
)

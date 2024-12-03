add_library(${PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC
    src/series.cpp
    src/quote.cpp
    src/asset.cpp
    src/ta.cpp
    src/config_parser.cpp

    src/screener/screener_method.cpp
    src/screener/data_method.cpp
    src/screener/value_method.cpp

    src/screener/all_of_filter.cpp
    src/screener/any_of_filter.cpp

    src/screener/crossover_filter.cpp
    src/screener/crossunder_filter.cpp

    src/screener/asset_data_provider.cpp

  INTERFACE
    FILE_SET HEADERS
      BASE_DIRS include/
      FILES
        include/pludux/asset.hpp
        include/pludux/quote.hpp
        include/pludux/series.hpp
        include/pludux/ta.hpp
        include/pludux/config_parser.hpp
        include/pludux/screener.hpp

        include/pludux/screener/screener_method.hpp 
        include/pludux/screener/data_method.hpp 
        include/pludux/screener/ta_with_period_method.hpp 
        include/pludux/screener/value_method.hpp 
        
        include/pludux/screener/screener_filter.hpp 
        include/pludux/screener/all_of_filter.hpp
        include/pludux/screener/any_of_filter.hpp
        include/pludux/screener/comparison_filter.hpp
        include/pludux/screener/crossover_filter.hpp
        include/pludux/screener/crossunder_filter.hpp

        include/pludux/screener/asset_data_provider.hpp
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

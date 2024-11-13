add_library(${PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC
    src/series.cpp
    src/quote.cpp
    src/asset.cpp
    src/ta.cpp

    src/screener/screener_method.cpp
    src/screener/field_method.cpp
    src/screener/sma_method.cpp
    src/screener/value_method.cpp

    src/screener/all_of_filter.cpp
    src/screener/any_of_filter.cpp
    src/screener/greater_than_filter.cpp
    src/screener/greater_equal_filter.cpp
    src/screener/less_than_filter.cpp
  INTERFACE
    FILE_SET HEADERS
      BASE_DIRS include/
      FILES
        include/pludux/asset.hpp
        include/pludux/quote.hpp
        include/pludux/series.hpp
        include/pludux/ta.hpp

        include/pludux/screener/screener_method.hpp 
        include/pludux/screener/field_method.hpp 
        include/pludux/screener/sma_method.hpp 
        include/pludux/screener/value_method.hpp 
        
        include/pludux/screener/screener_filter.hpp 
        include/pludux/screener/all_of_filter.hpp
        include/pludux/screener/any_of_filter.hpp
        include/pludux/screener/greater_than_filter.hpp
        include/pludux/screener/greater_equal_filter.hpp
        include/pludux/screener/less_than_filter.hpp

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

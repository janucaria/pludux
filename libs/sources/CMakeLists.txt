add_library(${PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC
    src/quote.cpp
    src/asset.cpp
  INTERFACE
    FILE_SET HEADERS
      BASE_DIRS include/
      FILES
        include/pludux/asset.hpp
        include/pludux/quote.hpp
        include/pludux/ta.hpp
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

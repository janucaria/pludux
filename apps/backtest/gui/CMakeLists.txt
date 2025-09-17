project(pludux-backtest-gui)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

if(NOT EMSCRIPTEN)

  set(PLUDUX_WEBGPU_DOWNLOAD_BUILD "release")

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PLUDUX_WEBGPU_DOWNLOAD_BUILD "debug")
  endif()

  FetchContent_Declare(
    wgpu_native
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL "https://github.com/gfx-rs/wgpu-native/releases/download/v25.0.2.2/wgpu-windows-x86_64-msvc-${PLUDUX_WEBGPU_DOWNLOAD_BUILD}.zip"
  )
  
  FetchContent_MakeAvailable(wgpu_native)

  add_library(webgpu SHARED IMPORTED)

  set_property(TARGET webgpu
    PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      "${wgpu_native_SOURCE_DIR}/include"
      "${wgpu_native_SOURCE_DIR}/include/webgpu"
  )
  
  set_target_properties(webgpu PROPERTIES
    IMPORTED_IMPLIB "${wgpu_native_SOURCE_DIR}/lib/wgpu_native.lib"
    IMPORTED_LOCATION "${wgpu_native_SOURCE_DIR}/lib/wgpu_native.dll"
  )  

  FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG        "3.4"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
  )
  set(GLFW_INSTALL OFF)
  set(GLFW_BUILD_DOCS OFF)
  
  FetchContent_Declare(
    nativefiledialog-extended
    GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git"
    GIT_TAG        v1.2.1
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
  )

  FetchContent_MakeAvailable(glfw nativefiledialog-extended)

endif()

FetchContent_Declare(
  glfw3webgpu
  GIT_REPOSITORY "https://github.com/eliemichel/glfw3webgpu.git"
  GIT_TAG        "v1.2.0"
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  PATCH_COMMAND 
    ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/patch/glfw3webgpu/
      <SOURCE_DIR>/
)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
  GIT_TAG        "v1.92.2b-docking"
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)

FetchContent_Declare(
  implot
  GIT_REPOSITORY "https://github.com/epezent/implot.git"
  GIT_TAG        "v0.16"
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  PATCH_COMMAND 
    ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/patch/implot/
      <SOURCE_DIR>/
)

FetchContent_MakeAvailable(glfw3webgpu imgui implot)

FetchContent_Declare(
  cereal
  GIT_REPOSITORY "https://github.com/USCiLab/cereal.git"
  GIT_TAG        "v1.3.2"
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
set(BUILD_DOC OFF)
set(BUILD_SANDBOX OFF)
set(SKIP_PERFORMANCE_COMPARISON ON)
set(THREAD_SAFE OFF)
set(JUST_INSTALL_CEREAL ON)

FetchContent_MakeAvailable(cereal)


add_executable(${PROJECT_NAME}

  src/main.cpp
  # backend files
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_wgpu.cpp
  # Dear ImGui files
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  # ImPlot files
  ${implot_SOURCE_DIR}/implot.cpp
  ${implot_SOURCE_DIR}/implot_items.cpp
)

target_sources(${PROJECT_NAME} PRIVATE
  FILE_SET
    CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES

    src/actions/change_strategy_json_action.cxx
    src/actions/load_asset_csv_action.cxx
    src/actions/load_backtests_setup_action.cxx
    src/actions.cxx
    
    src/app_state.cxx

    src/windows/assets_window.cxx
    src/windows/backtests_window.cxx
    src/windows/backtest_summary_window.cxx
    src/windows/dockspace_window.cxx
    src/windows/plot_data_window.cxx
    src/windows/profiles_window.cxx
    src/windows/strategies_window.cxx
    src/windows/trade_journal_window.cxx
    src/windows.cxx

    src/application.cxx
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  nssv_CONFIG_NO_EXCEPTIONS=1
  
  PLUDUX_VERSION="${CMAKE_PROJECT_VERSION}"
  PLUDUX_SOURCE_CODE_URL="${CMAKE_PROJECT_HOMEPAGE_URL}"
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${implot_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    pludux::backtest-lib
    glfw
    glfw3webgpu
    cereal::cereal
)

if(NOT EMSCRIPTEN)
  target_compile_definitions(${PROJECT_NAME} PUBLIC
    IMGUI_IMPL_WEBGPU_BACKEND_WGPU
  )

  target_link_libraries(${PROJECT_NAME}
    PUBLIC
      runtimeobject
      propsys
      ole32
      webgpu
      opengl32
      userenv
      ws2_32
      d3dcompiler
      ntdll
      kernel32
      nfd::nfd
  )

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
  )
  
else()
  target_compile_definitions(${PROJECT_NAME} PUBLIC
    IMGUI_DISABLE_FILE_FUNCTIONS=1
  )

  # add compiler flag -pthread
  target_compile_options(${PROJECT_NAME} PRIVATE
    "-fwasm-exceptions"
  )
  
  target_link_options(${PROJECT_NAME} PRIVATE
    "-fwasm-exceptions"

    "-sWASM=1"
    "-sUSE_WEBGPU=1"
    "-sUSE_GLFW=3"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sSTACK_SIZE=128kB"
    "-sEXPORTED_FUNCTIONS=[_main,stringToNewUTF8,UTF8ToString]"
  )
  
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "index")

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/web/index.html" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
  )

  
  install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "."
  )

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/index.html"
    "${CMAKE_CURRENT_BINARY_DIR}/index.wasm"
    "${CMAKE_SOURCE_DIR}/LICENSE.txt"
    DESTINATION "."
  )

endif()

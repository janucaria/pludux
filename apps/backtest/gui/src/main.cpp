#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <webgpu/webgpu.h>

#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>

#include "emscripten_js_imports.hpp"
#else
#include <glfw3webgpu.h>
#endif

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>
#include <implot.h>

#include "pludux_icon_font_data.h"
#include "ui/pludux_icons.hpp"

import pludux.apps.backtest;

#ifdef __EMSCRIPTEN__

extern "C" {

struct StdFreeDeleter {
  void operator()(void* ptr) const noexcept
  {
    std::free(ptr);
  }
};

EMSCRIPTEN_KEEPALIVE void
pludux_call_free_callback_2(void* user_function, void* arg1, void* arg2)
{
  using FunctionType = std::function<void(void*, void*)>;
  auto function_ptr = reinterpret_cast<FunctionType*>(user_function);
  auto unique_function = std::unique_ptr<FunctionType>(function_ptr);

  auto unique_arg1 = std::unique_ptr<void, StdFreeDeleter>{arg1};
  auto unique_arg2 = std::unique_ptr<void, StdFreeDeleter>{arg2};

  (*unique_function)(unique_arg1.get(), unique_arg2.get());
}

EMSCRIPTEN_KEEPALIVE void pludux_apps_backtest_js_opened_file_content_ready(
 char* name, char* data, void* user_callback, void* user_data)
{
  using JsOnOpenedFileContentReady =
   std::function<void(const std::string&, const std::string&, void*)>;

  const auto name_str = std::string(name);
  std::free(name);

  const auto data_str = std::string(data);
  std::free(data);

  const auto callback_ptr =
   reinterpret_cast<JsOnOpenedFileContentReady*>(user_callback);

  (*callback_ptr)(name_str, data_str, user_data);
}

EMSCRIPTEN_KEEPALIVE void pludux_apps_backtest_js_opened_file_text_ready(
 char* data, void* user_callback, void* user_data)
{
  using pludux::apps::ApplicationState;
  using JsOnPushOpenedFileAction =
   std::function<void(const std::string&, ApplicationState&)>;

  auto unique_data = std::unique_ptr<char, StdFreeDeleter>{data};

  auto data_str = std::string(reinterpret_cast<const char*>(unique_data.get()));

  auto callback_ptr =
   reinterpret_cast<JsOnPushOpenedFileAction*>(user_callback);

  auto& window_context =
   *reinterpret_cast<pludux::apps::WindowContext*>(user_data);
  window_context.push_action(
   [data_str = std::move(data_str), callback_ptr = callback_ptr](
    ApplicationState& app_state) { (*callback_ptr)(data_str, app_state); });
}
}

#endif

class Main {
public:
  Main()
  {
    initialized_ = initialize();
  }

  Main(const Main&) = delete;
  Main(Main&&) = delete;
  auto operator=(const Main&) -> Main& = delete;
  auto operator=(Main&&) -> Main& = delete;

  ~Main()
  {
    shutdown_ui();

    if(wgpu_surface_configured_) {
      wgpuSurfaceUnconfigure(wgpu_surface_);
    }
    if(wgpu_queue_) {
      wgpuQueueRelease(wgpu_queue_);
    }
    if(wgpu_surface_) {
      wgpuSurfaceRelease(wgpu_surface_);
    }
    if(wgpu_device_) {
      wgpuDeviceRelease(wgpu_device_);
    }
    if(wgpu_instance_) {
      wgpuInstanceRelease(wgpu_instance_);
    }
    if(window_) {
      glfwDestroyWindow(window_);
    }
    if(glfw_initialized_) {
      glfwTerminate();
    }
  }

  auto operator()() -> int
  {
    if(!initialized_) {
      std::cerr << "Could not initialize Pludux: " << initialization_error_
                << '\n';
      return EXIT_FAILURE;
    }

    if(!initialize_ui()) {
      std::cerr << "Could not initialize the user interface.\n";
      shutdown_ui();
      return EXIT_FAILURE;
    }

#ifndef __EMSCRIPTEN__
    glfwShowWindow(window_);
#endif

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(
     [](void* arg) {
       auto* app = static_cast<Main*>(arg);
       try {
         app->on_web_main_loop();
       } catch(const std::exception& ex) {
         app->enter_fatal_state(ex.what());
       } catch(...) {
         app->enter_fatal_state("unknown exception");
       }
     },
     this,
     0,
     1);

    return EXIT_SUCCESS;
#else
    show_loading_screen();
    auto application_started = false;
    auto result = EXIT_SUCCESS;
    try {
      application_.on_before_main_loop();
      application_started = true;

      while(!glfwWindowShouldClose(window_)) {
        on_main_loop();
      }
    } catch(const std::exception& ex) {
      std::cerr << "Fatal application error: " << ex.what() << '\n';
      result = EXIT_FAILURE;
    } catch(...) {
      std::cerr << "Fatal application error: unknown exception\n";
      result = EXIT_FAILURE;
    }

    if(application_started) {
      try {
        application_.on_after_main_loop();
      } catch(const std::exception& ex) {
        std::cerr << "Application shutdown failed: " << ex.what() << '\n';
        result = EXIT_FAILURE;
      } catch(...) {
        std::cerr << "Application shutdown failed: unknown exception\n";
        result = EXIT_FAILURE;
      }
    }

    shutdown_ui();
    return result;
#endif
  }

private:
  static constexpr auto initial_surface_width = 1280;
  static constexpr auto initial_surface_height = 720;

  int wgpu_surface_width_{initial_surface_width};
  int wgpu_surface_height_{initial_surface_height};

  WGPUInstance wgpu_instance_{};
  WGPUDevice wgpu_device_{};
  WGPUQueue wgpu_queue_{};
  WGPUSurface wgpu_surface_{};
  WGPUTextureFormat wgpu_preferred_fmt_{WGPUTextureFormat_Undefined};

  GLFWwindow* window_{};

  bool glfw_initialized_{};
  bool wgpu_surface_configured_{};
  bool imgui_context_created_{};
  bool implot_context_created_{};
  bool imgui_glfw_initialized_{};
  bool imgui_wgpu_initialized_{};
  bool initialized_{};
  bool fatal_state_entered_{};

#ifdef __EMSCRIPTEN__
  enum class WebStartupState {
    ShowLoadingScreen,
    InitializeApplication,
    RunApplication,
  };

  WebStartupState web_startup_state_{WebStartupState::ShowLoadingScreen};
  bool web_loading_overlay_hidden_{};
#endif

  std::string initialization_error_;
  pludux::apps::Application application_;

  auto initialize() -> bool
  {
    glfwSetErrorCallback([](int error, const char* description) {
      std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
    });

    if(!glfwInit()) {
      return fail_initialization("GLFW initialization failed");
    }
    glfw_initialized_ = true;

    // Make sure GLFW does not initialize any graphics context.
    // This needs to be done explicitly later.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifndef __EMSCRIPTEN__
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#endif
    window_ = glfwCreateWindow(
     wgpu_surface_width_, wgpu_surface_height_, "Pludux", nullptr, nullptr);
    if(!window_) {
      return fail_initialization("window creation failed");
    }

    glfwMaximizeWindow(window_);

#ifdef __EMSCRIPTEN__
    wgpu_instance_ = wgpuCreateInstance(nullptr);
#else
    wgpu_instance_ = wgpuCreateInstance(nullptr);
#endif
    if(!wgpu_instance_) {
      return fail_initialization("WebGPU instance creation failed");
    }

    wgpu_surface_ = create_window_wgpu_surface(wgpu_instance_);
    if(!wgpu_surface_) {
      return fail_initialization("WebGPU surface creation failed");
    }

#ifdef __EMSCRIPTEN__
    wgpu_device_ = emscripten_webgpu_get_device();
    if(!wgpu_device_) {
      return fail_initialization("WebGPU device acquisition failed");
    }

    if(!select_surface_format(nullptr)) {
      return false;
    }
#else
    auto options = WGPURequestAdapterOptions{};
    options.compatibleSurface = wgpu_surface_;
    auto adapter = request_adapter(options);
    if(!adapter) {
      return fail_initialization("WebGPU adapter acquisition failed");
    }

    wgpu_device_ = request_device(adapter);
    const auto format_selected = select_surface_format(adapter);
    wgpuAdapterRelease(adapter);
    if(!wgpu_device_) {
      return fail_initialization("WebGPU device acquisition failed");
    }
    if(!format_selected) {
      return false;
    }
#endif

    wgpu_queue_ = wgpuDeviceGetQueue(wgpu_device_);
    if(!wgpu_queue_) {
      return fail_initialization("WebGPU queue acquisition failed");
    }

    return configure_webgpu_surface();
  }

  auto fail_initialization(std::string reason) -> bool
  {
    initialization_error_ = std::move(reason);
    return false;
  }

  auto initialize_ui() -> bool
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imgui_context_created_ = true;
    ImPlot::CreateContext();
    implot_context_created_ = true;

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    // Base font, then merge in the Lucide icon font so icon glyphs can be
    // used inline with regular text (e.g. PLUDUX_ICON_DELETE " Delete").
    // Both fonts need an explicit matching reference size for MergeMode.
    // Loaded from an in-memory byte array (see CMakeLists.txt) rather than
    // AddFontFromFileTTF because the Emscripten build disables ImGui's
    // file-based font loading (IMGUI_DISABLE_FILE_FUNCTIONS).
    const auto icon_font_size = 13.0f;
    auto default_font_config = ImFontConfig{};
    default_font_config.SizePixels = icon_font_size;
    io.Fonts->AddFontDefaultBitmap(&default_font_config);

    auto icon_font_config = ImFontConfig{};
    icon_font_config.MergeMode = true;
    icon_font_config.PixelSnapH = true;
    icon_font_config.FontDataOwnedByAtlas = false;
    icon_font_config.GlyphMinAdvanceX = icon_font_size;
    icon_font_config.GlyphOffset = ImVec2{0.0f, 3.0f};

    static const auto icon_font_ranges =
     std::array<ImWchar, 3>{PLUDUX_ICON_MIN, PLUDUX_ICON_MAX_16, 0};

    io.Fonts->AddFontFromMemoryTTF(
     const_cast<unsigned char*>(pludux_icon_font_data),
     static_cast<int>(pludux_icon_font_data_size),
     icon_font_size,
     &icon_font_config,
     icon_font_ranges.data());

    if(!ImGui_ImplGlfw_InitForOther(window_, true)) {
      return false;
    }
    imgui_glfw_initialized_ = true;
    io.IniFilename = nullptr;

#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window_, "#canvas");
#endif

    auto init_info = ImGui_ImplWGPU_InitInfo{};
    init_info.Device = wgpu_device_;
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = wgpu_preferred_fmt_;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    if(!ImGui_ImplWGPU_Init(&init_info)) {
      return false;
    }
    imgui_wgpu_initialized_ = true;

    return true;
  }

  void shutdown_ui() noexcept
  {
    if(imgui_wgpu_initialized_) {
      ImGui_ImplWGPU_Shutdown();
      imgui_wgpu_initialized_ = false;
    }
    if(imgui_glfw_initialized_) {
      ImGui_ImplGlfw_Shutdown();
      imgui_glfw_initialized_ = false;
    }
    if(implot_context_created_) {
      ImPlot::DestroyContext();
      implot_context_created_ = false;
    }
    if(imgui_context_created_) {
      ImGui::DestroyContext();
      imgui_context_created_ = false;
    }
  }

  void enter_fatal_state(const std::string& reason)
  {
    if(fatal_state_entered_) {
      return;
    }
    fatal_state_entered_ = true;
    std::cerr << "Fatal main-loop error: " << reason << '\n';

#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#else
    glfwSetWindowShouldClose(window_, GLFW_TRUE);
#endif
  }

  void on_main_loop()
  {
    glfwPollEvents();

#ifndef __EMSCRIPTEN__
    if(glfwGetWindowAttrib(window_, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      return;
    }
#endif

    render_frame([this] {
      auto window_width = 0;
      auto window_height = 0;
      glfwGetWindowSize(window_, &window_width, &window_height);

      application_.set_window_size(window_width, window_height);
      application_.on_update();
    });
  }

#ifdef __EMSCRIPTEN__
  void on_web_main_loop()
  {
    switch(web_startup_state_) {
    case WebStartupState::ShowLoadingScreen:
      show_loading_screen();
      web_startup_state_ = WebStartupState::InitializeApplication;
      return;

    case WebStartupState::InitializeApplication:
      application_.on_before_main_loop();
      web_startup_state_ = WebStartupState::RunApplication;
      return;

    case WebStartupState::RunApplication:
      on_main_loop();
      if(!web_loading_overlay_hidden_) {
        pludux_js_hide_loading_overlay();
        web_loading_overlay_hidden_ = true;
      }
      return;
    }
  }
#endif

  void show_loading_screen()
  {
    glfwPollEvents();

    // A surface may report itself as temporarily suboptimal immediately after
    // a maximized window is first shown. Retrying lets the loading frame be
    // presented after the surface has reconfigured to its actual size.
    for(auto attempt = 0; attempt < 3; ++attempt) {
      if(render_frame([] { draw_loading_screen(); })) {
        break;
      }
      glfwPollEvents();
    }
  }

  static void draw_loading_screen()
  {
    constexpr auto background_color = IM_COL32(14, 16, 20, 255);
    constexpr auto title_color = IM_COL32(238, 241, 247, 255);
    constexpr auto detail_color = IM_COL32(151, 160, 176, 255);
    constexpr auto spinner_track_color = IM_COL32(48, 54, 66, 255);
    constexpr auto spinner_color = IM_COL32(94, 129, 244, 255);
    constexpr auto pi = 3.14159265358979323846f;

    auto* viewport = ImGui::GetMainViewport();
    const auto center = ImVec2{viewport->Pos.x + viewport->Size.x * 0.5f,
                               viewport->Pos.y + viewport->Size.y * 0.5f};
    auto* draw_list = ImGui::GetBackgroundDrawList(viewport);
    draw_list->AddRectFilled(
     viewport->Pos,
     ImVec2{viewport->Pos.x + viewport->Size.x,
            viewport->Pos.y + viewport->Size.y},
     background_color);

    constexpr auto title = "PLUDUX";
    constexpr auto title_font_size = 28.0f;
    auto* font = ImGui::GetFont();
    const auto title_size =
     font->CalcTextSizeA(title_font_size, 1000.0f, 0.0f, title);
    draw_list->AddText(
     font,
     title_font_size,
     ImVec2{center.x - title_size.x * 0.5f, center.y - 58.0f},
     title_color,
     title);

    constexpr auto detail = "Preparing your workspace...";
    const auto detail_size = ImGui::CalcTextSize(detail);
    draw_list->AddText(
     ImVec2{center.x - detail_size.x * 0.5f, center.y + 35.0f},
     detail_color,
     detail);

    const auto spinner_center = ImVec2{center.x, center.y};
    constexpr auto spinner_radius = 11.0f;
    draw_list->AddCircle(
     spinner_center, spinner_radius, spinner_track_color, 32, 3.0f);

    const auto start_angle =
     std::fmod(static_cast<float>(ImGui::GetTime()) * 4.0f, 2.0f * pi);
    draw_list->PathArcTo(
     spinner_center,
     spinner_radius,
     start_angle,
     start_angle + pi * 1.35f,
     24);
    draw_list->PathStroke(spinner_color, 0, 3.0f);
  }

  template<typename DrawContent>
  auto render_frame(DrawContent&& draw_content) -> bool
  {
    // React to changes in screen size.
    auto width = wgpu_surface_width_;
    auto height = wgpu_surface_height_;
    glfwGetFramebufferSize(window_, &width, &height);
    if(width <= 0 || height <= 0) {
      return false;
    }
    if(width != wgpu_surface_width_ || height != wgpu_surface_height_) {
      ImGui_ImplWGPU_InvalidateDeviceObjects();
      if(!configure_webgpu_surface(width, height) ||
         !ImGui_ImplWGPU_CreateDeviceObjects()) {
        throw std::runtime_error("could not resize the WebGPU surface");
      }
    }

    auto surface_texture = WGPUSurfaceTexture{};
    wgpuSurfaceGetCurrentTexture(wgpu_surface_, &surface_texture);
    struct SurfaceTextureReleaser {
      WGPUTexture texture;

      ~SurfaceTextureReleaser()
      {
        if(texture) {
          wgpuTextureRelease(texture);
        }
      }
    };
    const auto surface_texture_releaser =
     SurfaceTextureReleaser{surface_texture.texture};

    if(ImGui_ImplWGPU_IsSurfaceStatusError(surface_texture.status)) {
      throw std::runtime_error(
       "unrecoverable WebGPU surface error (status " +
       std::to_string(surface_texture.status) + ")");
    }
    if(ImGui_ImplWGPU_IsSurfaceStatusSubOptimal(surface_texture.status)) {
      configure_webgpu_surface(width, height);
      return false;
    }

    // Start the Dear ImGui frame
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    std::forward<DrawContent>(draw_content)();

    // Rendering
    ImGui::Render();

    auto tex_view_descriptor = WGPUTextureViewDescriptor{};
    tex_view_descriptor.label = {"ImGui Surface Texture View", WGPU_STRLEN};
    tex_view_descriptor.format = wgpuTextureGetFormat(surface_texture.texture);
    tex_view_descriptor.dimension = WGPUTextureViewDimension_2D;
    tex_view_descriptor.baseMipLevel = 0;
    tex_view_descriptor.mipLevelCount = 1;
    tex_view_descriptor.baseArrayLayer = 0;
    tex_view_descriptor.arrayLayerCount = 1;
    tex_view_descriptor.aspect = WGPUTextureAspect_All;
    auto color_tex_view =
     wgpuTextureCreateView(surface_texture.texture, &tex_view_descriptor);

    auto color_attachments = WGPURenderPassColorAttachment{};
    color_attachments.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    color_attachments.loadOp = WGPULoadOp_Clear;
    color_attachments.storeOp = WGPUStoreOp_Store;
    color_attachments.clearValue = WGPUColor{0.055f, 0.063f, 0.078f, 1.00f};
    color_attachments.view = color_tex_view;

    auto render_pass_desc = WGPURenderPassDescriptor{};
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachments;
    render_pass_desc.depthStencilAttachment = nullptr;

    auto enc_desc = WGPUCommandEncoderDescriptor{};
    enc_desc.label = {"ImGui Command Encoder", WGPU_STRLEN};
    WGPUCommandEncoder encoder =
     wgpuDeviceCreateCommandEncoder(wgpu_device_, &enc_desc);

    WGPURenderPassEncoder pass =
     wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);

    auto cmd_buffer_desc = WGPUCommandBufferDescriptor{};
    cmd_buffer_desc.label = {"ImGui Command Buffer", WGPU_STRLEN};

    WGPUCommandBuffer cmd_buffer =
     wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);
    wgpuCommandEncoderRelease(encoder);

    wgpuQueueSubmit(wgpu_queue_, 1, &cmd_buffer);
    wgpuCommandBufferRelease(cmd_buffer);

    wgpuTextureViewRelease(color_attachments.view);

#ifndef __EMSCRIPTEN__
    wgpuSurfacePresent(wgpu_surface_);
#endif
    return true;
  }

  auto configure_webgpu_surface() -> bool
  {
    return configure_webgpu_surface(
     wgpu_surface_width_, wgpu_surface_height_);
  }

  auto configure_webgpu_surface(int width, int height) -> bool
  {
    if(width <= 0 || height <= 0) {
      return false;
    }

    if(wgpu_surface_configured_) {
      wgpuSurfaceUnconfigure(wgpu_surface_);
    }

    wgpu_surface_width_ = width;
    wgpu_surface_height_ = height;

    auto surface_config = WGPUSurfaceConfiguration{};
    surface_config.format = wgpu_preferred_fmt_;
    surface_config.viewFormatCount = 0;
    surface_config.viewFormats = nullptr;
    surface_config.width = wgpu_surface_width_;
    surface_config.height = wgpu_surface_height_;
    surface_config.usage = WGPUTextureUsage_RenderAttachment;
    surface_config.device = wgpu_device_;
    surface_config.presentMode = WGPUPresentMode_Fifo;
    surface_config.alphaMode = WGPUCompositeAlphaMode_Auto;

    wgpuSurfaceConfigure(wgpu_surface_, &surface_config);
    wgpu_surface_configured_ = true;
    return true;
  }

  auto select_surface_format(WGPUAdapter adapter) -> bool
  {
    auto capabilities = WGPUSurfaceCapabilities{};
    wgpuSurfaceGetCapabilities(wgpu_surface_, adapter, &capabilities);
    if(capabilities.formatCount == 0 || !capabilities.formats) {
      wgpuSurfaceCapabilitiesFreeMembers(capabilities);
      return fail_initialization("WebGPU surface exposes no texture formats");
    }

    // Keep ImGui's colors in the same linear UNORM space used before surface
    // capability detection was added. Picking an sRGB format here changes the
    // apparent contrast of the entire UI and makes some theme fills harder to
    // read.
    wgpu_preferred_fmt_ = capabilities.formats[0];
    for(auto index = std::size_t{}; index < capabilities.formatCount; ++index) {
      const auto format = capabilities.formats[index];
      if(format == WGPUTextureFormat_BGRA8Unorm) {
        wgpu_preferred_fmt_ = format;
        break;
      }
      if(format == WGPUTextureFormat_RGBA8Unorm) {
        wgpu_preferred_fmt_ = format;
      }
    }
    wgpuSurfaceCapabilitiesFreeMembers(capabilities);
    return true;
  }

#ifndef __EMSCRIPTEN__
  template<typename Handle>
  struct RequestState {
    Handle result{};
    std::atomic_bool completed{};
  };

  template<typename Handle>
  static auto await_request(RequestState<Handle>& state) -> Handle
  {
    while(!state.completed.load(std::memory_order_acquire)) {
      state.completed.wait(false, std::memory_order_acquire);
    }
    return state.result;
  }

  auto request_adapter(const WGPURequestAdapterOptions& options) const
   -> WGPUAdapter
  {
    auto state = RequestState<WGPUAdapter>{};
    auto callback_info = WGPURequestAdapterCallbackInfo{};
    callback_info.mode = WGPUCallbackMode_AllowSpontaneous;
    callback_info.callback = [](WGPURequestAdapterStatus status,
                                WGPUAdapter result,
                                WGPUStringView message,
                                void* userdata1,
                                void*) {
      auto& state = *static_cast<RequestState<WGPUAdapter>*>(userdata1);
      if(status == WGPURequestAdapterStatus_Success) {
        state.result = result;
      } else {
        std::cerr << "Could not get WebGPU adapter: "
                  << std::string(message.data, message.length) << '\n';
      }
      state.completed.store(true, std::memory_order_release);
      state.completed.notify_one();
    };
    callback_info.userdata1 = &state;

    wgpuInstanceRequestAdapter(wgpu_instance_, &options, callback_info);
    return await_request(state);
  }

  auto request_device(WGPUAdapter adapter) const -> WGPUDevice
  {
    auto descriptor = WGPUDeviceDescriptor{};
    descriptor.label = {"Pludux Device", WGPU_STRLEN};
    descriptor.uncapturedErrorCallbackInfo.callback = on_webgpu_error;
    descriptor.deviceLostCallbackInfo.callback = on_webgpu_device_lost;

    auto state = RequestState<WGPUDevice>{};
    auto callback_info = WGPURequestDeviceCallbackInfo{};
    callback_info.mode = WGPUCallbackMode_AllowSpontaneous;
    callback_info.callback = [](WGPURequestDeviceStatus status,
                                WGPUDevice result,
                                WGPUStringView message,
                                void* userdata1,
                                void*) {
      auto& state = *static_cast<RequestState<WGPUDevice>*>(userdata1);
      if(status == WGPURequestDeviceStatus_Success) {
        state.result = result;
      } else {
        std::cerr << "Could not get WebGPU device: "
                  << std::string(message.data, message.length) << '\n';
      }
      state.completed.store(true, std::memory_order_release);
      state.completed.notify_one();
    };
    callback_info.userdata1 = &state;

    wgpuAdapterRequestDevice(adapter, &descriptor, callback_info);
    return await_request(state);
  }

  static void on_webgpu_error(WGPUDevice const*,
                              WGPUErrorType error_type,
                              WGPUStringView message,
                              void*,
                              void*)
  {
    std::cerr << "WebGPU " << ImGui_ImplWGPU_GetErrorTypeName(error_type)
              << " error: "
              << std::string(message.data, message.length) << '\n';
  }

  static void on_webgpu_device_lost(WGPUDevice const*,
                                    WGPUDeviceLostReason reason,
                                    WGPUStringView message,
                                    void*,
                                    void*)
  {
    std::cerr << "WebGPU device lost ("
              << ImGui_ImplWGPU_GetDeviceLostReasonName(reason) << "): "
              << std::string(message.data, message.length) << '\n';
  }
#endif

  auto create_window_wgpu_surface(WGPUInstance instance) -> WGPUSurface
  {
#ifdef __EMSCRIPTEN__
    auto canvas_source = WGPUEmscriptenSurfaceSourceCanvasHTMLSelector{};
    canvas_source.chain.sType =
     WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
    canvas_source.selector = {"#canvas", WGPU_STRLEN};
    canvas_source.chain.next = nullptr;

    auto surface_descriptor = WGPUSurfaceDescriptor{};
    surface_descriptor.nextInChain = &canvas_source.chain;

    return wgpuInstanceCreateSurface(instance, &surface_descriptor);

#else
    return glfwCreateWindowWGPUSurface(instance, window_);
#endif
  }
};

auto main(int argc, char** argv) -> int
{
  static_cast<void>(argc);
  static_cast<void>(argv);

#ifdef __EMSCRIPTEN__
  // make the main instance static to avoid it being destroyed
  static
#endif
   Main main_instance;

  return main_instance();
}

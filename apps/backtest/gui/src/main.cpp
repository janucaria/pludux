#include <cstdio>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>

#include <implot.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#else
#include <webgpu/wgpu.h>
#endif

#include <glfw3webgpu.h>

#include "./app_state.hpp"

#ifdef __EMSCRIPTEN__
#include "./serialization.hpp"
#endif

import pludux.apps.backtest.application;

#ifdef __EMSCRIPTEN__

extern "C" {

EMSCRIPTEN_KEEPALIVE void pludux_apps_backtest_open_file(char* data,
                                                         void* app_state_ptr)
{
  using pludux::apps::AppState;

  const auto data_str = std::string(data);
  std::free(data);

  auto app_state = *reinterpret_cast<AppState*>(app_state_ptr);

  app_state.push_action([data_str](pludux::apps::AppStateData& state) {
    auto in_stream = std::istringstream{data_str};

    if(!in_stream.good()) {
      const auto error_message =
       std::format("Failed to open data stream for reading.");
      throw std::runtime_error(error_message);
    }

    auto in_archive = cereal::JSONInputArchive(in_stream);

    in_archive(cereal::make_nvp("pludux", state));
  });
}

EMSCRIPTEN_KEEPALIVE void
pludux_apps_backtest_load_csv_asset(char* name, char* data, void* app_state_ptr)
{
  using pludux::apps::AppState;
  using pludux::apps::LoadAssetCsvStrAction;

  auto name_str = std::string(name);
  std::free(name);

  auto data_str = std::string(data);
  std::free(data);

  auto app_state = *reinterpret_cast<AppState*>(app_state_ptr);

  app_state.emplace_action<LoadAssetCsvStrAction>(std::move(name_str),
                                                  std::move(data_str));
}

EMSCRIPTEN_KEEPALIVE void pludux_apps_backtest_change_strategy_json_str(
 char* name, char* data, void* app_state_ptr)
{
  using pludux::apps::AppState;
  using pludux::apps::ChangeStrategyJsonStrAction;

  auto name_str = std::string(name);
  std::free(name);

  auto data_str = std::string(data);
  std::free(data);

  auto app_state = *reinterpret_cast<AppState*>(app_state_ptr);

  app_state.emplace_action<ChangeStrategyJsonStrAction>(std::move(name_str),
                                                        std::move(data_str));
}

EMSCRIPTEN_KEEPALIVE void
pludux_apps_backtest_load_backtests_setup(char* data, void* app_state_ptr)
{
  using pludux::apps::AppState;
  using pludux::apps::LoadBacktestsSetupStingAction;

  auto data_str = std::string(data);
  std::free(data);

  auto app_state = *reinterpret_cast<AppState*>(app_state_ptr);
  app_state.emplace_action<LoadBacktestsSetupStingAction>(std::move(data_str));
}
}

#endif

class Main {
public:
  Main()
  : wgpu_surface_width_{1280}
  , wgpu_surface_height_{720}
  {
    glfwSetErrorCallback([](int error, const char* description) {
      printf("GLFW Error %d: %s\n", error, description);
    });

    if(!glfwInit()) {
      return;
    }

    // Make sure GLFW does not initialize any graphics context.
    // This needs to be done explicitly later.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window_ = glfwCreateWindow(
     wgpu_surface_width_, wgpu_surface_height_, "Pludux", nullptr, nullptr);
    if(!window_) {
      return;
    }

    glfwMaximizeWindow(window_);

    auto instance = wgpuCreateInstance(nullptr);

    wgpu_surface_ = glfwCreateWindowWGPUSurface(instance, window_);
    if(!wgpu_surface_) {
      wgpuInstanceRelease(instance);
      return;
    }

#ifdef __EMSCRIPTEN__
    wgpu_preferred_fmt_ = wgpuSurfaceGetPreferredFormat(wgpu_surface_, nullptr);
    wgpu_device_ = emscripten_webgpu_get_device();
#else
    wgpu_preferred_fmt_ = WGPUTextureFormat_BGRA8Unorm;

    auto adapter = WGPUAdapter{};

    auto options = WGPURequestAdapterOptions{};
    options.compatibleSurface = wgpu_surface_;

    auto adapter_callback_info = WGPURequestAdapterCallbackInfo{};
    adapter_callback_info.mode = WGPUCallbackMode_WaitAnyOnly;
    adapter_callback_info.callback = [](WGPURequestAdapterStatus status,
                                        WGPUAdapter adapter,
                                        WGPUStringView message,
                                        void* userdata1,
                                        void* userdata2) {
      if(status == WGPURequestAdapterStatus_Success) {
        *reinterpret_cast<WGPUAdapter*>(userdata1) = adapter;
      } else {
        std::cerr << "Could not get WebGPU adapter: "
                  << std::string(message.data, message.length) << std::endl;
      }
    };
    adapter_callback_info.userdata1 = reinterpret_cast<void*>(&adapter);
    adapter_callback_info.userdata2 = nullptr;

    auto request_adapter_future =
     wgpuInstanceRequestAdapter(instance, &options, adapter_callback_info);

    if(!adapter) {
      wgpuInstanceRelease(instance);
      return;
    }

    auto device_desc = WGPUDeviceDescriptor{};
    device_desc.label = {"Pludux Device", WGPU_STRLEN};
    device_desc.uncapturedErrorCallbackInfo.callback =
     [](WGPUDevice const* device,
        WGPUErrorType error_type,
        WGPUStringView message,
        void* userdata1,
        void* userdata2) {
       auto error_type_lbl = std::string{};
       switch(error_type) {
       case WGPUErrorType_Validation:
         error_type_lbl = "Validation";
         break;
       case WGPUErrorType_OutOfMemory:
         error_type_lbl = "Out of memory";
         break;
       case WGPUErrorType_Internal:
         error_type_lbl = "Internal";
         break;
       case WGPUErrorType_Unknown:
         error_type_lbl = "Unknown";
         break;
       case WGPUErrorType_Force32:
         error_type_lbl = "Force32";
         break;
       default:
         error_type_lbl = "Not detected";
         break;
       }

       const auto error_message = std::string(message.data, message.length);
       std::cerr << "WebGPU error: " << error_type_lbl << " - " << error_message
                 << std::endl;
     };

    device_desc.deviceLostCallbackInfo.callback =
     [](WGPUDevice const* device,
        WGPUDeviceLostReason reason,
        WGPUStringView message,
        void* userdata1,
        void* userdata2) {
       auto reason_label = std::string{};
       switch(reason) {
       case WGPUDeviceLostReason_Unknown:
         reason_label = "Unknown";
         break;
       case WGPUDeviceLostReason_Destroyed:
         reason_label = "Destroyed";
         break;
       case WGPUDeviceLostReason_Force32:
         reason_label = "Force32";
         break;
       default:
         reason_label = "Not detected";
       }

       const auto message_str = std::string(message.data, message.length);
       std::cerr << "WebGPU device lost: " << reason_label << " - "
                 << message_str << std::endl;
     };

    auto device_callback_info = WGPURequestDeviceCallbackInfo{};
    device_callback_info.mode = WGPUCallbackMode_WaitAnyOnly;
    device_callback_info.callback = [](WGPURequestDeviceStatus status,
                                       WGPUDevice device,
                                       WGPUStringView message,
                                       void* userdata1,
                                       void* userdata2) {
      if(status == WGPURequestDeviceStatus_Success) {
        *reinterpret_cast<WGPUDevice*>(userdata1) = device;
      } else {
        std::cerr << "Could not get WebGPU device: "
                  << std::string(message.data, message.length) << std::endl;
      }
    };
    device_callback_info.userdata1 = reinterpret_cast<void*>(&wgpu_device_);

    auto request_device_future =
     wgpuAdapterRequestDevice(adapter, &device_desc, device_callback_info);

    wgpuAdapterRelease(adapter);

#endif

    if(!wgpu_device_) {
      wgpuInstanceRelease(instance);
      return;
    }

    configure_webgpu_surface();
  }

  ~Main()
  {
    wgpuSurfaceUnconfigure(wgpu_surface_);
    wgpuSurfaceRelease(wgpu_surface_);
    wgpuDeviceRelease(wgpu_device_);

    glfwDestroyWindow(window_);
    glfwTerminate();
  }

  auto operator()(int argc, char** argv) -> int
  {
    glfwShowWindow(window_);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window_, true);
#ifdef __EMSCRIPTEN__
    io.IniFilename = nullptr;
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window_, "#canvas");
#else
    const auto temp_dir = std::filesystem::temp_directory_path();
    const auto ini_file = temp_dir / "imgui.ini";
    ini_file_ = ini_file.generic_string();

    io.IniFilename = ini_file_.c_str();
#endif
    auto init_info = ImGui_ImplWGPU_InitInfo{};
    init_info.Device = wgpu_device_;
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = wgpu_preferred_fmt_;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    application_.on_before_main_loop();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(
     [](void* arg) {
       auto app = reinterpret_cast<Main*>(arg);
       app->on_main_loop();
     },
     this,
     0,
     1);
#else

    while(!glfwWindowShouldClose(window_)) {
      on_main_loop();
    }

#endif

    application_.on_after_main_loop();

    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    return 0;
  }

private:
  int wgpu_surface_width_;
  int wgpu_surface_height_;

  std::string ini_file_;

  WGPUDevice wgpu_device_;
  WGPUSurface wgpu_surface_;
  WGPUTextureFormat wgpu_preferred_fmt_;

  GLFWwindow* window_;

  pludux::apps::Application application_;

  void on_main_loop()
  {
    glfwPollEvents();

#ifndef __EMSCRIPTEN__
    if(glfwGetWindowAttrib(window_, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      return;
    }
#endif

    // React to changes in screen size
    auto width = wgpu_surface_width_;
    auto height = wgpu_surface_height_;
    glfwGetFramebufferSize(window_, &width, &height);
    if(width != wgpu_surface_width_ || height != wgpu_surface_height_) {
      ImGui_ImplWGPU_InvalidateDeviceObjects();
      change_wgpu_surface_size(width, height);
      ImGui_ImplWGPU_CreateDeviceObjects();
    }

    // Start the Dear ImGui frame
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    auto window_width = 0;
    auto window_height = 0;
    glfwGetWindowSize(window_, &window_width, &window_height);

    application_.set_window_size(window_width, window_height);
    application_.on_update();

    ImGui::EndFrame();

    // Rendering
    ImGui::Render();

    auto surface_texture = WGPUSurfaceTexture{};
    wgpuSurfaceGetCurrentTexture(wgpu_surface_, &surface_texture);

#ifdef __EMSCRIPTEN__
    if(surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
      return;
    }
#else
    if(surface_texture.status !=
       WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
      return;
    }
#endif

    auto tex_view_descriptor = WGPUTextureViewDescriptor{};
#ifdef __EMSCRIPTEN__
    tex_view_descriptor.label = "ImGui Surface Texture View";
#else
    tex_view_descriptor.label = {"ImGui Surface Texture View", WGPU_STRLEN};
#endif
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
    color_attachments.clearValue = WGPUColor{0.45f, 0.55f, 0.60f, 1.00f};
    color_attachments.view = color_tex_view;

    auto render_pass_desc = WGPURenderPassDescriptor{};
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachments;
    render_pass_desc.depthStencilAttachment = nullptr;

    auto enc_desc = WGPUCommandEncoderDescriptor{};
#ifdef __EMSCRIPTEN__
    enc_desc.label = "ImGui Command Encoder";
#else
    enc_desc.label = {"ImGui Command Encoder", WGPU_STRLEN};
#endif
    WGPUCommandEncoder encoder =
     wgpuDeviceCreateCommandEncoder(wgpu_device_, &enc_desc);

    WGPURenderPassEncoder pass =
     wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);

    auto cmd_buffer_desc = WGPUCommandBufferDescriptor{};

#ifdef __EMSCRIPTEN__
    cmd_buffer_desc.label = "ImGui Command Buffer";
#else
    cmd_buffer_desc.label = {"ImGui Command Buffer", WGPU_STRLEN};
#endif

    WGPUCommandBuffer cmd_buffer =
     wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);
    wgpuCommandEncoderRelease(encoder);

    auto wgpu_queue = wgpuDeviceGetQueue(wgpu_device_);
    wgpuQueueSubmit(wgpu_queue, 1, &cmd_buffer);
    wgpuCommandBufferRelease(cmd_buffer);

    wgpuTextureViewRelease(color_attachments.view);

#ifndef __EMSCRIPTEN__
    wgpuSurfacePresent(wgpu_surface_);

    wgpuDevicePoll(wgpu_device_, false, nullptr);
#endif

    wgpuQueueRelease(wgpu_queue);
  }

  void configure_webgpu_surface()
  {
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
  }

  void change_wgpu_surface_size(int width, int height)
  {
    wgpu_surface_width_ = width;
    wgpu_surface_height_ = height;

    wgpuSurfaceUnconfigure(wgpu_surface_);
    configure_webgpu_surface();
  }
};

auto main(int argc, char** argv) -> int
{
#ifdef __EMSCRIPTEN__
  // make the main instance static to avoid it being destroyed
  static
#endif
   Main main_instance;

  return main_instance(argc, argv);
}

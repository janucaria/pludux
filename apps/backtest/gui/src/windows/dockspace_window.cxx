module;

#include <algorithm>
#include <array>
#include <ctime>
#include <format>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <stop_token>
#include <string>

#ifdef __EMSCRIPTEN__
#include "../emscripten_js_imports.hpp"
#else
#include <nfd.hpp>
#endif

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <imgui_internal.h>

export module pludux.apps.backtest:windows.dockspace_window;

import :application_state;
import :serialization;
import :window_context;

export namespace pludux::apps {

class DockspaceWindow {
public:
  void render(this DockspaceWindow& self, WindowContext& context)
  {
    const auto* viewport = ImGui::GetMainViewport();
    const auto window_flags =
     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
     ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
    ImGui::Begin("Pludux Workspace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    self.render_toolbar(context);
    constexpr auto workspace_gap = 2.0f;
    const auto current_y = ImGui::GetCursorPosY();
    const auto default_gap = ImGui::GetStyle().ItemSpacing.y;
    ImGui::SetCursorPosY(current_y - default_gap + workspace_gap);
    self.render_dockspace(ImGui::GetContentRegionAvail());

    ImGui::End();
    self.render_about_popup();
  }

private:
  bool open_about_popup_{false};
  bool desktop_layout_initialized_{false};

  static auto toolbar_label(const char* icon, const char* text, const char* id)
   -> std::string
  {
    return std::format("{} {}##{}", icon, text, id);
  }

  void render_toolbar(this DockspaceWindow& self, WindowContext& context)
  {
    const auto toolbar_padding = ImVec2{12.0f, 8.0f};
    const auto toolbar_height =
     ImGui::GetFrameHeight() + (2.0f * toolbar_padding.y);

    ImGui::PushStyleColor(ImGuiCol_ChildBg,
                          ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
    ImGui::PushStyleColor(ImGuiCol_Border,
                          ImGui::GetStyleColorVec4(ImGuiCol_Separator));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, toolbar_padding);
    ImGui::BeginChild(
     "Workspace Toolbar",
     ImVec2{0.0f, toolbar_height},
     ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(PLUDUX_ICON_CATEGORY_PRICE_VOLUME
                           "  Pludux Backtest");
    ImGui::SameLine(0.0f, 18.0f);

    if(ImGui::Button(
        toolbar_label(PLUDUX_ICON_IMPORT, "Open", "open").c_str())) {
      self.open_application(context);
    }
    ImGui::SameLine();
    if(ImGui::Button(toolbar_label(PLUDUX_ICON_SAVE, "Save", "save").c_str())) {
      self.save_application(context);
    }

    ImGui::SameLine(0.0f, 18.0f);
    ImGui::BeginDisabled(!context.has_undo());
    if(ImGui::Button(toolbar_label(PLUDUX_ICON_UNDO, "Undo", "undo").c_str())) {
      context.push_undo();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!context.has_redo());
    if(ImGui::Button(toolbar_label(PLUDUX_ICON_REDO, "Redo", "redo").c_str())) {
      context.push_redo();
    }
    ImGui::EndDisabled();

    const auto about_label = PLUDUX_ICON_INFO " About";
    const auto about_width = ImGui::CalcTextSize(about_label).x +
                             (2.0f * ImGui::GetStyle().FramePadding.x);
    ImGui::SameLine();
    const auto about_x =
     ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - about_width;
    ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), about_x));
    if(ImGui::Button(about_label)) {
      self.open_about_popup_ = true;
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
  }

  void render_dockspace(this DockspaceWindow& self, ImVec2 size)
  {
    static constexpr auto dockspace_flags = ImGuiDockNodeFlags_None;
    const auto dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, size, dockspace_flags);

    if(self.desktop_layout_initialized_) {
      return;
    }
    self.desktop_layout_initialized_ = true;
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id,
                              dockspace_flags | ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, size);

    auto dock_left_id = dockspace_id;
    auto dock_right_id = ImGui::DockBuilderSplitNode(
     dock_left_id, ImGuiDir_Right, 0.3f, nullptr, &dock_left_id);
    auto dock_left_down_id = ImGui::DockBuilderSplitNode(
     dock_left_id, ImGuiDir_Down, 0.3f, nullptr, &dock_left_id);
    auto dock_right_down_id = ImGui::DockBuilderSplitNode(
     dock_right_id, ImGuiDir_Down, 0.3f, nullptr, &dock_right_id);

    ImGui::DockBuilderDockWindow("Plots", dock_left_id);
    ImGui::DockBuilderDockWindow("Trades", dock_left_down_id);
    ImGui::DockBuilderDockWindow("Overview", dock_right_id);
    for(const auto* window : {"Backtests",
                              "Assets",
                              "Strategies",
                              "Markets",
                              "Brokers",
                              "Profiles"}) {
      ImGui::DockBuilderDockWindow(window, dock_right_down_id);
    }
    ImGui::DockBuilderFinish(dockspace_id);
  }

  void open_application(this DockspaceWindow&, WindowContext& context)
  {
#ifdef __EMSCRIPTEN__
    context.request_discard_all_drafts();
    using Callback = std::function<void(const std::string&, ApplicationState&)>;
    static const auto callback =
     Callback{[](const std::string& file_data, ApplicationState& app_state) {
       auto in_stream = std::istringstream{file_data};
       if(!in_stream.good()) {
         throw std::runtime_error("Failed to open data stream for reading.");
       }
       app_state = load_application_state_json(in_stream);
     }};
    pludux_js_open_single_text_file(".pludux", &callback, &context);
#else
    auto nfd_guard = NFD::Guard{};
    auto in_path = NFD::UniquePath{};
    const auto filter_item =
     std::array<nfdfilteritem_t, 1>{{"Pludux Files", "pludux"}};
    const auto result =
     NFD::OpenDialog(in_path, filter_item.data(), filter_item.size());
    if(result == NFD_OKAY) {
      context.request_discard_all_drafts();
      const auto selected_path = std::string{in_path.get()};
      context.push_action([selected_path](ApplicationState& app_state) {
        auto in_stream = std::ifstream{selected_path};
        if(!in_stream.is_open()) {
          throw std::runtime_error(
           std::format("Failed to open '{}' for reading.", selected_path));
        }
        app_state = load_application_state_json(in_stream);
        const auto& settings = app_state.imgui_ini_settings();
        ImGui::LoadIniSettingsFromMemory(settings.c_str(), settings.size());
      });
    } else if(result == NFD_ERROR) {
      throw std::runtime_error(
       std::format("Error '{}': {}", "Open", NFD::GetError()));
    }
#endif
  }

  void save_application(this DockspaceWindow&, WindowContext& context)
  {
    context.update_imgui_ini_settings();
    auto out_stream = std::ostringstream{};
    save_application_state_json(out_stream, context.app_state());
    const auto content = out_stream.str();
    const auto file_name =
     "pludux-backtest-" + std::to_string(std::time(nullptr)) + ".pludux";
#ifdef __EMSCRIPTEN__
    pludux_js_save_file(file_name.c_str(), content.c_str(), "application/json");
#else
    auto nfd_guard = NFD::Guard{};
    auto out_path = NFD::UniquePath{};
    const auto filter_item =
     std::array<nfdfilteritem_t, 1>{{"Pludux Files", "pludux"}};
    const auto result =
     NFD::SaveDialog(out_path, filter_item.data(), filter_item.size());
    if(result == NFD_OKAY) {
      const auto saved_path = std::string{out_path.get()};
      context.push_action([saved_path, content](ApplicationState&) {
        auto file = std::ofstream{saved_path};
        if(!file.is_open()) {
          throw std::runtime_error(
           std::format("Failed to open '{}' for writing.", saved_path));
        }
        file << content;
      });
    } else if(result == NFD_ERROR) {
      throw std::runtime_error(
       std::format("Error '{}': {}", "Save", NFD::GetError()));
    }
#endif
  }

  void render_about_popup(this DockspaceWindow& self)
  {
    if(self.open_about_popup_) {
      ImGui::OpenPopup("About");
    }

    const auto* viewport = ImGui::GetMainViewport();
    const auto popup_width = std::min(480.0f, viewport->WorkSize.x - 40.0f);
    const auto popup_center =
     ImVec2{viewport->WorkPos.x + (viewport->WorkSize.x * 0.5f),
            viewport->WorkPos.y + (viewport->WorkSize.y * 0.5f)};
    ImGui::SetNextWindowPos(
     popup_center, ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});
    ImGui::SetNextWindowSizeConstraints(
     ImVec2{popup_width, 0.0f},
     ImVec2{popup_width, viewport->WorkSize.y - 40.0f});

    if(ImGui::BeginPopupModal(
        "About", &self.open_about_popup_, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::TextUnformatted("Pludux Backtest");
      ImGui::Separator();
      ImGui::Text("Version: %s", PLUDUX_VERSION);
      ImGui::TextWrapped("Source: %s", PLUDUX_SOURCE_CODE_URL);

#ifdef __EMSCRIPTEN__
      pludux_js_set_body_cursor(ImGui::IsItemHovered());
      if(ImGui::IsItemClicked()) {
        ImGui::CloseCurrentPopup();
        pludux_js_open_url(PLUDUX_SOURCE_CODE_URL);
      }
#endif
      ImGui::TextWrapped("AGPL licensed. Copyright (c) 2026 Januar Andaria.");
      ImGui::TextWrapped("Full licence text included in the LICENSE.txt file.");
      if(ImGui::Button("Close")) {
        self.open_about_popup_ = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }
};

} // namespace pludux::apps

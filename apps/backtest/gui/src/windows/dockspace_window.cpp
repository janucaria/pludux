#include <ctime>
#include <format>
#include <memory>
#include <ranges>
#include <string>

#ifdef __EMSCRIPTEN__
#include <cstdlib>

#include <emscripten.h>
#include <emscripten/val.h>
#else
#include <nfd.hpp>
#endif

#include <imgui.h>
#include <imgui_internal.h>

#include "../app_state.hpp"

#include "./dockspace_window.hpp"

namespace pludux::apps {

DockspaceWindow::DockspaceWindow()
: open_about_popup_{false}
{
}

void DockspaceWindow::render(AppState& app_state)
{
  const auto& state = app_state.state();

  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  const auto* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  auto window_flags =
   ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  if(dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
    window_flags |= ImGuiWindowFlags_NoBackground;
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", nullptr, window_flags);
  ImGui::PopStyleVar();

  ImGui::PopStyleVar(2);

  // Submit the DockSpace
  const auto dockspace_id = ImGui::GetID("MainDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

  static auto first_time = true;
  if(first_time) {
    first_time = false;
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id,
                              dockspace_flags | ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    auto dock_left_id = dockspace_id;
    auto dock_right_id = ImGui::DockBuilderSplitNode(
     dock_left_id, ImGuiDir_Right, 0.3f, nullptr, &dock_left_id);
    auto dock_left_down_id = ImGui::DockBuilderSplitNode(
     dock_left_id, ImGuiDir_Down, 0.3f, nullptr, &dock_left_id);
    auto dock_right_down_id = ImGui::DockBuilderSplitNode(
     dock_right_id, ImGuiDir_Down, 0.3f, nullptr, &dock_right_id);

    ImGui::DockBuilderDockWindow("Charts", dock_left_id);
    ImGui::DockBuilderDockWindow("Trades", dock_left_down_id);
    ImGui::DockBuilderDockWindow("Summary", dock_right_id);
    ImGui::DockBuilderDockWindow("Assets", dock_right_down_id);

    ImGui::DockBuilderFinish(dockspace_id);
  }

  if(ImGui::BeginMenuBar()) {
    if(ImGui::BeginMenu("File")) {
      constexpr auto menu_item_open_config = "Open Strategy (JSON)";
      if(ImGui::MenuItem(menu_item_open_config)) {
#ifdef __EMSCRIPTEN__

        auto new_app_state_ptr =
         std::make_unique<AppState>(app_state).release();

        EM_ASM(
         {
           var file_selector = document.createElement('input');
           file_selector.type = 'file';
           file_selector.onchange = function(event)
           {
             var file = event.target.files[0];

             var reader = new FileReader();
             reader.onload = function(event)
             {
               var file_name = file.name;
               var data = event.target.result;
               var decoder = new TextDecoder('utf-8');
               var decoded_data = decoder.decode(data);

               var name_ptr = Module.stringToNewUTF8(file_name);
               var data_ptr = Module.stringToNewUTF8(decoded_data);

               // call the C++ function
               Module._pludux_apps_backtest_change_strategy_json_str(
                name_ptr, data_ptr, $0);
             };

             reader.readAsArrayBuffer(file);
           };
           file_selector.accept = '.json';
           file_selector.click();
         },
         new_app_state_ptr);
#else
        NFD::Guard nfdGuard;
        NFD::UniquePath outPath;
        nfdfilteritem_t filterItem[1] = {{"JSON Files", "json"}};
        nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1);

        if(result == NFD_OKAY) {
          const auto selected_path = std::string(outPath.get());

          app_state.emplace_action<ChangeStrategyJsonFileAction>(selected_path);

        } else if(result == NFD_CANCEL) {
        } else {
          const auto error_message = std::format(
           "Error '{}': {}", menu_item_open_config, NFD::GetError());
          throw std::runtime_error(error_message);
        }
#endif
      }

      constexpr auto menu_item_open_csv = "Add Data (CSV)";
      if(ImGui::MenuItem(
          menu_item_open_csv, nullptr, false, state.strategy.has_value())) {
#ifdef __EMSCRIPTEN__

        auto new_app_state_ptr =
         std::make_unique<AppState>(app_state).release();

        EM_ASM(
         {
           var file_selector = document.createElement('input');
           file_selector.type = 'file';
           file_selector.onchange = function(event)
           {
             var file = event.target.files[0];

               var reader = new FileReader();
               reader.onload = function(event)
               {
                 var file_name = file.name;
                 var data = event.target.result;
                 var decoder = new TextDecoder('utf-8');
                 var decoded_data = decoder.decode(data);

                 // transfer the data to the C++ side
                 var name_ptr = Module.stringToNewUTF8(file_name);
                 var data_ptr = Module.stringToNewUTF8(decoded_data);

                 // call the C++ function
                 Module._pludux_apps_backtest_load_csv_asset(
                  name_ptr, data_ptr, $0);
               };

               reader.readAsArrayBuffer(file);
           };
           file_selector.accept = '.csv';
           file_selector.click();
         },
         new_app_state_ptr);

#else
        NFD::Guard nfdGuard;
        NFD::UniquePath outPath;
        nfdfilteritem_t filterItem[1] = {{"CSV Files", "csv"}};
        nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1);

        if(result == NFD_OKAY) {
          const auto selected_path = std::string(outPath.get());

          app_state.emplace_action<LoadAssetCsvFileAction>(selected_path);

        } else if(result == NFD_CANCEL) {
        } else {
          const auto error_message =
           std::format("Error '{}': {}", menu_item_open_csv, NFD::GetError());
          throw std::runtime_error(error_message);
        }
#endif
      }
      ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Help")) {
      if(ImGui::MenuItem("About")) {
        open_about_popup_ = true;
      }

      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }

  ImGui::End();

  if(open_about_popup_) {
    ImGui::OpenPopup("About");
    if(ImGui::BeginPopupModal(
        "About", &open_about_popup_, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("%s", "Pludux Backtest");
      ImGui::Separator();
      ImGui::Text("%s", std::format("Version: {}", PLUDUX_VERSION).c_str());
      ImGui::Text("%s", "Source Code: ");
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(0, 1, 1, 1), "%s", PLUDUX_SOURCE_CODE_URL);

#ifdef __EMSCRIPTEN__
      EM_ASM(
       { document.body.style.cursor = $0 ? 'pointer' : 'default'; },
       ImGui::IsItemHovered());
      if(ImGui::IsItemClicked()) {
        ImGui::CloseCurrentPopup();
        EM_ASM(
         {
           var url = Module.UTF8ToString($0);
           window.open(url, '_blank');
         },
         PLUDUX_SOURCE_CODE_URL);
      }
#endif

      ImGui::Separator();
      ImGui::Text("%s", "This software is licensed under the MIT License.");
      ImGui::Text("%s", "Copyright (c) 2025 Januar Andaria");
      ImGui::Text("%s", "Full licence text included in the LICENSE.txt file.");

      if(ImGui::Button("OK")) {
        ImGui::CloseCurrentPopup();
        open_about_popup_ = false;
      }
      ImGui::EndPopup();
    }
  }
}

} // namespace pludux::apps

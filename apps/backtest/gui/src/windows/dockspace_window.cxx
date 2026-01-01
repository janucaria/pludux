module;

#include <array>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
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

#include <cereal/cereal.hpp>
#include <jsoncons/json.hpp>
#include <rapidcsv.h>

#include <cereal/archives/json.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

export module pludux.apps.backtest:windows.dockspace_window;

import :application_state;
import :window_context;
import :serialization;

export namespace pludux::apps {

class DockspaceWindow {
public:
  DockspaceWindow()
  : open_about_popup_{false}
  {
  }

  void render(this DockspaceWindow& self, WindowContext& context)
  {
    auto& open_about_popup = self.open_about_popup_;

    const auto& app_state = context.app_state();

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
      ImGui::DockBuilderDockWindow("Backtests", dock_right_down_id);
      ImGui::DockBuilderDockWindow("Assets", dock_right_down_id);
      ImGui::DockBuilderDockWindow("Strategies", dock_right_down_id);
      ImGui::DockBuilderDockWindow("Markets", dock_right_down_id);
      ImGui::DockBuilderDockWindow("Brokers", dock_right_down_id);
      ImGui::DockBuilderDockWindow("Profiles", dock_right_down_id);

      ImGui::DockBuilderFinish(dockspace_id);
    }

    if(ImGui::BeginMenuBar()) {
      if(ImGui::BeginMenu("File")) {
        {
          constexpr auto menu_item_open = "Open";
          if(ImGui::MenuItem(menu_item_open)) {
#ifdef __EMSCRIPTEN__

            EM_ASM(
             {
               var fileSelector = document.createElement('input');
               fileSelector.type = 'file';
               fileSelector.accept = '.pludux-json';
               fileSelector.onchange = function(event)
               {
                 var file = event.target.files[0];
                 if(!file) {
                   return;
                 }

                 var reader = new FileReader();
                 reader.onload = function(event)
                 {
                   var data = event.target.result;
                   var decoder = new TextDecoder('utf-8');
                   var decodedData = decoder.decode(data);

                   // transfer the data to the C++ side
                   var data_ptr = Module.stringToNewUTF8(decodedData);

                   // call the C++ function
                   Module._pludux_apps_backtest_open_file(data_ptr, $0);
                 };

                 reader.readAsArrayBuffer(file);
               };
               fileSelector.click();
             },
             &context);

#else
            auto nfd_guard = NFD::Guard{};
            auto in_path = NFD::UniquePath{};
            const auto filter_item = std::array<nfdfilteritem_t, 1>{
             {"Pludux JSON Files", "pludux-json"}};
            auto result =
             NFD::OpenDialog(in_path, filter_item.data(), filter_item.size());
            if(result == NFD_OKAY) {
              const auto selected_path = std::string(in_path.get());
              context.push_action([selected_path](ApplicationState& app_state) {
                auto in_stream = std::ifstream{selected_path};

                if(!in_stream.is_open()) {
                  const auto error_message = std::format(
                   "Failed to open '{}' for reading.", selected_path);
                  throw std::runtime_error(error_message);
                }

                auto in_archive = cereal::JSONInputArchive(in_stream);

                auto loaded_state = ApplicationState{};
                in_archive(cereal::make_nvp("pludux", loaded_state));
                app_state = std::move(loaded_state);
              });
            } else if(result == NFD_CANCEL) {
              // User cancelled the open dialog
            } else {
              const auto error_message =
               std::format("Error '{}': {}", menu_item_open, NFD::GetError());
              throw std::runtime_error(error_message);
            }
#endif
          }
        }

        {
#ifdef __EMSCRIPTEN__
          constexpr auto menu_item_save_as = "Save";
          if(ImGui::MenuItem(menu_item_save_as)) {
            auto out_stream = std::ostringstream{};
            auto out_archive = cereal::JSONOutputArchive(out_stream);
            out_archive(cereal::make_nvp("pludux", context.app_state()));

            // TODO: bug in Cereal not adding the close object at the end when
            // using stringstream
            out_stream << "}\n";

            const auto out_str = out_stream.str();
            EM_ASM(
             {
               var fileSave = document.createElement('a');
               fileSave.download = 'saved.pludux-json';
               fileSave.style.display = 'none';
               var data = new Blob([Module.UTF8ToString($0)], {
                 type:
                   'application/json'
               });
               fileSave.href = URL.createObjectURL(data);
               fileSave.click();
               URL.revokeObjectURL(fileSave.href);
             },
             out_str.c_str());
          }
#else
          constexpr auto menu_item_save_as = "Save As...";
          if(ImGui::MenuItem(menu_item_save_as)) {
            auto nfd_guard = NFD::Guard{};
            auto out_path = NFD::UniquePath{};

            const auto filter_item = std::array<nfdfilteritem_t, 1>{
             {"Pludux JSON Files", "pludux-json"}};

            auto result =
             NFD::SaveDialog(out_path, filter_item.data(), filter_item.size());

            if(result == NFD_OKAY) {
              const auto saved_path = std::string(out_path.get());
              context.push_action([saved_path](ApplicationState& app_state) {
                auto out_stream = std::ofstream{saved_path};

                if(!out_stream.is_open()) {
                  const auto error_message =
                   std::format("Failed to open '{}' for writing.", saved_path);
                  throw std::runtime_error(error_message);
                }

                auto out_archive = cereal::JSONOutputArchive(out_stream);

                out_archive(cereal::make_nvp("pludux", app_state));
              });
            } else if(result == NFD_CANCEL) {
              // User cancelled the save dialog
            } else {
              const auto error_message = std::format(
               "Error '{}': {}", menu_item_save_as, NFD::GetError());
              throw std::runtime_error(error_message);
            }
          }
#endif
        }

        constexpr auto menu_item_open_config = "Load Strategies (JSON)";
        if(ImGui::MenuItem(menu_item_open_config)) {
#ifdef __EMSCRIPTEN__
          EM_ASM(
           {
             var fileSelector = document.createElement('input');
             fileSelector.type = 'file';
             fileSelector.multiple = true;
             fileSelector.onchange = function(event)
             {
               for(var i = 0; i < event.target.files.length; i++) {
                 var file = event.target.files[i];

                 var reader = new FileReader();
                 reader.onload = function(event)
                 {
                   var reader = event.target;
                   var fileName = reader.onload.prototype.fileName;
                   var data = reader.result;
                   var decoder = new TextDecoder('utf-8');
                   var decodedData = decoder.decode(data);

                   // transfer the data to the C++ side
                   var name_ptr = Module.stringToNewUTF8(fileName);
                   var data_ptr = Module.stringToNewUTF8(decodedData);

                   // call the C++ function
                   Module._pludux_apps_backtest_load_strategy_json_str(
                    name_ptr, data_ptr, $0);
                 };
                 reader.onload.prototype.fileName = file.name;

                 reader.readAsArrayBuffer(file);
               }
             };
             fileSelector.accept = '.json';
             fileSelector.click();
           },
           &context);
#else
          auto nfd_guard = NFD::Guard{};
          auto out_paths = NFD::UniquePathSet{};

          const auto filter_item =
           std::array<nfdfilteritem_t, 1>{{"JSON Files", "json"}};
          auto result = NFD::OpenDialogMultiple(
           out_paths, filter_item.data(), filter_item.size());

          if(result == NFD_OKAY) {
            auto paths_count = nfdpathsetsize_t{};

            result = NFD::PathSet::Count(out_paths, paths_count);
            if(result == NFD_ERROR) {
              const auto error_message = std::format(
               "Error '{}': {}", menu_item_open_config, NFD::GetError());
              throw std::runtime_error(error_message);
            }

            for(nfdpathsetsize_t i = 0; i < paths_count; ++i) {
              auto out_path = NFD::UniquePathSetPath{};
              result = NFD::PathSet::GetPath(out_paths, i, out_path);

              if(result == NFD_ERROR) {
                const auto error_message = std::format(
                 "Error '{}': {}", menu_item_open_config, NFD::GetError());
                throw std::runtime_error(error_message);
              }

              const auto selected_path = std::string(out_path.get());
              context.push_action(LoadStrategyJsonAction{selected_path});
            }

          } else if(result == NFD_CANCEL) {
          } else {
            const auto error_message = std::format(
             "Error '{}': {}", menu_item_open_config, NFD::GetError());
            throw std::runtime_error(error_message);
          }
#endif
        }

        constexpr auto menu_item_open_csv = "Add or Replace Assets (CSV)";
        if(ImGui::MenuItem(menu_item_open_csv)) {
#ifdef __EMSCRIPTEN__

          EM_ASM(
           {
             var fileSelector = document.createElement('input');
             fileSelector.type = 'file';
             fileSelector.multiple = true;
             fileSelector.onchange = function(event)
             {
               for(var i = 0; i < event.target.files.length; i++) {
                 var file = event.target.files[i];

                 var reader = new FileReader();
                 reader.onload = function(event)
                 {
                   var reader = event.target;
                   var fileName = reader.onload.prototype.fileName;
                   var data = reader.result;
                   var decoder = new TextDecoder('utf-8');
                   var decodedData = decoder.decode(data);

                   // transfer the data to the C++ side
                   var name_ptr = Module.stringToNewUTF8(fileName);
                   var data_ptr = Module.stringToNewUTF8(decodedData);

                   // call the C++ function
                   Module._pludux_apps_backtest_load_csv_asset(
                    name_ptr, data_ptr, $0);
                 };
                 reader.onload.prototype.fileName = file.name;

                 reader.readAsArrayBuffer(file);
               }
             };
             fileSelector.accept = '.csv';
             fileSelector.click();
           },
           &context);

#else
          auto nfd_guard = NFD::Guard{};
          auto out_paths = NFD::UniquePathSet{};

          constexpr auto filter_item =
           std::array<nfdfilteritem_t, 1>{{"CSV Files", "csv"}};

          auto result = NFD::OpenDialogMultiple(
           out_paths, filter_item.data(), filter_item.size());

          if(result == NFD_OKAY) {
            auto paths_count = nfdpathsetsize_t{};

            result = NFD::PathSet::Count(out_paths, paths_count);
            if(result == NFD_ERROR) {
              const auto error_message = std::format(
               "Error '{}': {}", menu_item_open_csv, NFD::GetError());
              throw std::runtime_error(error_message);
            }

            for(nfdpathsetsize_t i = 0; i < paths_count; ++i) {
              auto out_path = NFD::UniquePathSetPath{};
              result = NFD::PathSet::GetPath(out_paths, i, out_path);

              if(result == NFD_ERROR) {
                const auto error_message = std::format(
                 "Error '{}': {}", menu_item_open_csv, NFD::GetError());
                throw std::runtime_error(error_message);
              }

              const auto selected_path = std::string(out_path.get());
              context.push_action(LoadAssetCsvAction{selected_path});
            }

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
          open_about_popup = true;
        }

        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

    ImGui::End();

    if(open_about_popup) {
      ImGui::OpenPopup("About");
      if(ImGui::BeginPopupModal(
          "About", &open_about_popup, ImGuiWindowFlags_AlwaysAutoResize)) {
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
        ImGui::Text("%s", "This software is licensed under the AGPL License.");
        ImGui::Text("%s", "Copyright (c) 2025 Januar Andaria");
        ImGui::Text("%s",
                    "Full licence text included in the LICENSE.txt file.");

        if(ImGui::Button("OK")) {
          ImGui::CloseCurrentPopup();
          open_about_popup = false;
        }
        ImGui::EndPopup();
      }
    }
  }

private:
  bool open_about_popup_;
};

} // namespace pludux::apps

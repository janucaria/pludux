module;

#include <array>
#include <format>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>

#ifdef __EMSCRIPTEN__
#include <cstdlib>

#include <emscripten.h>
#include <emscripten/val.h>
#else
#include <nfd.hpp>
#endif

#include <imgui.h>

export module pludux.apps.backtest:windows.assets_window;

import :window_context;

export namespace pludux::apps {

class AssetsWindow {
public:
  void render(this const auto, WindowContext& context)
  {
    const auto& assets = context.assets();

    ImGui::Begin("Assets", nullptr);

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!assets.empty()) {
      for(auto i = 0; i < assets.size(); ++i) {
        const auto& asset = *assets[i];
        const auto& asset_name = asset.name();

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();

        ImGui::Text("%s", asset_name.c_str());

        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
        if(ImGui::Button("Delete")) {
          context.push_action([i](ApplicationState& app_state) {
            app_state.remove_asset_at_index(i);
          });
        }

        ImGui::PopID();
      }
    }

    ImGui::EndChild();
    if(ImGui::Button("Load Assets")) {
#ifdef __EMSCRIPTEN__

      using JsOnOpenedFileContentReady =
       std::function<void(const std::string&, const std::string&, void*)>;

      static const auto callback =
       JsOnOpenedFileContentReady{[](const std::string& file_name,
                                     const std::string& file_data,
                                     void* user_data) {
         auto& context = *reinterpret_cast<WindowContext*>(user_data);

         auto action = LoadAssetCsvAction{file_name, file_data};
         context.push_action(std::move(action));
       }};

      EM_ASM(
       {
         var fileSelector = document.createElement('input');
         fileSelector.type = 'file';
         fileSelector.accept = '.csv';
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
               Module._pludux_apps_backtest_js_opened_file_content_ready(
                name_ptr, data_ptr, $0, $1);
             };
             reader.onload.prototype.fileName = file.name;

             reader.readAsArrayBuffer(file);
           }
         };
         fileSelector.click();
       },
       &callback,
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
          const auto error_message =
           std::format("Error '{}': {}", "Load Asset", NFD::GetError());
          throw std::runtime_error(error_message);
        }

        for(nfdpathsetsize_t i = 0; i < paths_count; ++i) {
          auto out_path = NFD::UniquePathSetPath{};
          result = NFD::PathSet::GetPath(out_paths, i, out_path);

          if(result == NFD_ERROR) {
            const auto error_message =
             std::format("Error '{}': {}", "Load Asset", NFD::GetError());
            throw std::runtime_error(error_message);
          }

          const auto selected_path = std::string(out_path.get());
          context.push_action(LoadAssetCsvAction{selected_path});
        }

      } else if(result == NFD_CANCEL) {
      } else {
        const auto error_message =
         std::format("Error '{}': {}", "Load Asset", NFD::GetError());
        throw std::runtime_error(error_message);
      }

#endif
    }

    ImGui::EndGroup();

    ImGui::End();
  }
};

} // namespace pludux::apps

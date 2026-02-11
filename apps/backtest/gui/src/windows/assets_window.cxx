module;

#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef __EMSCRIPTEN__
#include <cstdlib>

#include <emscripten.h>
#include <emscripten/val.h>
#else
#include <nfd.hpp>
#endif

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.assets_window;

import :window_context;

export namespace pludux::apps {

class AssetsWindow {
public:
  AssetsWindow()
  : current_page_(AssetPage::List)
  , selected_asset_ptr_{nullptr}
  , editing_asset_ptr_{nullptr}
  {
#ifdef __EMSCRIPTEN__
    EM_ASM(
     {
       const inputElementId = UTF8ToString($0);
       if(!document.getElementById(inputElementId)) {
         var fileInput = document.createElement('input');
         fileInput.type = 'file';
         fileInput.id = inputElementId;
         fileInput.accept = '.csv';
         fileInput.style.display = 'none';
         document.body.appendChild(fileInput);
       }
     },
     input_element_id_);
#endif
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Assets", nullptr);

    switch(self.current_page_) {
    case AssetPage::AddNewAsset:
      self.render_add_new_asset(context);
      break;
    case AssetPage::EditAsset:
      self.render_edit_asset(context);
      break;
    case AssetPage::List:
    default:
      self.render_assets_list(context);
      break;
    }

    ImGui::End();
  }

private:
  static constexpr auto input_element_id_ =
   "pludux-backtest-gui-input-asset-file";

  enum class AssetPage { List, AddNewAsset, EditAsset } current_page_;

  std::shared_ptr<backtest::Asset> selected_asset_ptr_;
  std::shared_ptr<backtest::Asset> editing_asset_ptr_;

  void render_assets_list(this auto& self, WindowContext& context)
  {
    const auto& assets = context.assets();

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!assets.empty()) {
      for(auto i = 0; i < assets.size(); ++i) {
        const auto& asset = assets[i];

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();

        ImGui::Text("%s", asset->name().c_str());

        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        if(ImGui::Button("Edit")) {
          self.current_page_ = AssetPage::EditAsset;
          self.selected_asset_ptr_ = asset;
          self.editing_asset_ptr_ =
           std::make_shared<backtest::Asset>(*self.selected_asset_ptr_);
        }

        ImGui::SameLine();

        if(ImGui::Button("Delete")) {
          context.push_action([i](ApplicationState& app_state) {
            app_state.remove_asset_at_index(i);
          });
        }

        ImGui::PopID();
      }
    }

    ImGui::EndChild();
    if(ImGui::Button("Add New Asset")) {
      self.current_page_ = AssetPage::AddNewAsset;

      self.selected_asset_ptr_ = nullptr;
      self.editing_asset_ptr_ = std::make_shared<backtest::Asset>();
    }

    ImGui::SameLine();

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
  }

  void render_add_new_asset(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Asset");
    ImGui::Separator();

    self.edit_asset_form();

    ImGui::EndChild();

    if(ImGui::Button("OK")) {
      self.submit_asset_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::EndGroup();
  }

  void render_edit_asset(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Asset");
    ImGui::Separator();

    self.edit_asset_form();

    ImGui::EndChild();

    const auto equivalent_asset =
     self.selected_asset_ptr_->equivalent_with_nans_as_equal(
      *self.editing_asset_ptr_);

    if(ImGui::Button("OK")) {
      self.submit_asset_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::SameLine();
    ImGui::BeginDisabled(equivalent_asset);
    if(ImGui::Button("Apply")) {
      self.submit_asset_changes(context);
    }
    ImGui::EndDisabled();

    ImGui::EndGroup();
  }

  void edit_asset_form(this auto& self)
  {
    {
      auto asset_name = self.editing_asset_ptr_->name();
      ImGui::InputText("Name", &asset_name);
      self.editing_asset_ptr_->name(asset_name);
    }

    auto field_resolver = self.editing_asset_ptr_->field_resolver();
    {
      auto open_field = field_resolver.open_field();
      ImGui::InputText("Open Field", &open_field);
      field_resolver.open_field(open_field);
    }
    {
      auto high_field = field_resolver.high_field();
      ImGui::InputText("High Field", &high_field);
      field_resolver.high_field(high_field);
    }
    {
      auto low_field = field_resolver.low_field();
      ImGui::InputText("Low Field", &low_field);
      field_resolver.low_field(low_field);
    }
    {
      auto close_field = field_resolver.close_field();
      ImGui::InputText("Close Field", &close_field);
      field_resolver.close_field(close_field);
    }
    {
      auto volume_field = field_resolver.volume_field();
      ImGui::InputText("Volume Field", &volume_field);
      field_resolver.volume_field(volume_field);
    }
    {
      if(ImGui::Button("Select CSV File")) {
#ifdef __EMSCRIPTEN__
        EM_ASM(
         {
           var fileSelector = document.getElementById(UTF8ToString($0));

           fileSelector.onchange = function(event)
           {
             var file = fileSelector.files[0];

             var reader = new FileReader();
             reader.onload = function(event)
             {
               var reader = event.target;
               var fileName = reader.onload.prototype.fileName;
               var data = reader.result;
               var decoder = new TextDecoder('utf-8');
               var decodedData = decoder.decode(data);

               var namePtr = Module.stringToNewUTF8(fileName);
               var dataPtr = Module.stringToNewUTF8(decodedData);

               Module._pludux_call_free_callback_2($1, namePtr, dataPtr);

               fileSelector.value = "";
             };
             reader.onload.prototype.fileName = file.name;

             reader.readAsArrayBuffer(file);
           };

           fileSelector.click();
         },
         input_element_id_,
         std::make_unique<std::function<void(void*, void*)>>(
          [edit_asset_ptr = self.editing_asset_ptr_](void* name_ptr,
                                                     void* data_ptr) mutable {
            auto name_cstr = reinterpret_cast<char*>(name_ptr);
            auto asset_name = std::string(name_cstr);

            auto data_cstr = reinterpret_cast<char*>(data_ptr);
            auto asset_data = std::string(data_cstr);

            auto data_stream = std::istringstream{asset_data};
            pludux::update_asset_from_csv(*edit_asset_ptr, data_stream);

            if(edit_asset_ptr->name().empty()) {
              edit_asset_ptr->name(asset_name);
            }
          })
          .release());
#else

        auto nfd_guard = NFD::Guard{};
        auto in_path = NFD::UniquePath{};
        const auto filter_item =
         std::array<nfdfilteritem_t, 1>{{"Pludux CSV Files", "csv"}};
        auto result =
         NFD::OpenDialog(in_path, filter_item.data(), filter_item.size());
        if(result == NFD_OKAY) {
          const auto selected_path = std::string(in_path.get());
          auto csv_stream = std::ifstream{selected_path};
          if(!csv_stream.is_open()) {
            const auto error_message =
             std::format("Failed to open file: {}", selected_path);
            throw std::runtime_error(error_message);
          }

          pludux::update_asset_from_csv(*self.editing_asset_ptr_, csv_stream);

          if(self.editing_asset_ptr_->name().empty()) {
            self.editing_asset_ptr_->name(
             std::filesystem::path{selected_path}.stem().string());
          }
        } else if(result == NFD_CANCEL) {
          // User cancelled the open dialog
        } else {
          const auto error_message =
           std::format("Error '{}': {}", "Open CSV file", NFD::GetError());
          throw std::runtime_error(error_message);
        }
#endif
      }
    }

    self.editing_asset_ptr_->field_resolver(field_resolver);
  }

  void submit_asset_changes(this auto& self, WindowContext& context)
  {
    context.push_action(
     [asset_ptr = self.selected_asset_ptr_,
      edit_asset_ptr = self.editing_asset_ptr_](ApplicationState& app_state) {
       if(edit_asset_ptr->name().empty()) {
         edit_asset_ptr->name("Unnamed");
       }

       if(asset_ptr == nullptr) {
         app_state.add_asset(
          std::make_shared<backtest::Asset>(std::move(*edit_asset_ptr)));
         return;
       }

       const auto reset_backtests =
        !asset_ptr->equivalent_rules(*edit_asset_ptr);
       if(reset_backtests) {
         for(auto& backtest : app_state.backtests()) {
           if(backtest->asset_ptr() == asset_ptr) {
             backtest->reset();
           }
         }
       }

       *asset_ptr = *edit_asset_ptr;
     });
  }

  void reset(this auto& self)
  {
    self.current_page_ = AssetPage::List;
    self.selected_asset_ptr_ = nullptr;
    self.editing_asset_ptr_ = nullptr;
  }
};

} // namespace pludux::apps

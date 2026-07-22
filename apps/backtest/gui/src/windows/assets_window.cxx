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
#include "../emscripten_js_imports.hpp"
#else
#include <nfd.hpp>
#endif

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.assets_window;

import :window_context;
import :ui.widgets;

export namespace pludux::apps {

class AssetsWindow {
public:
  AssetsWindow()
  : current_page_(AssetPage::List)
  , selected_asset_handle_opt_{std::nullopt}
  , editing_asset_ptr_{nullptr}
  {
#ifdef __EMSCRIPTEN__
    pludux_js_ensure_hidden_file_input(input_element_id_, ".csv");
#endif
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Assets");

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

  void discard_draft(this AssetsWindow& self) noexcept
  {
    self.reset();
  }

private:
  static constexpr auto input_element_id_ =
   "pludux-backtest-gui-input-asset-file";

  enum class AssetPage { List, AddNewAsset, EditAsset } current_page_;

  std::optional<backtest::AssetStoreHandle> selected_asset_handle_opt_;
  std::shared_ptr<backtest::Asset> editing_asset_ptr_;
  ImGuiTextFilter asset_filter_;
  ui::DraftAction selected_draft_action_{ui::DraftAction::Apply};

  void render_assets_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& asset_handles = app_state.get_asset_handles();
    const auto backtest_ptr = app_state.selected_backtest_if_present();

    ImGui::BeginGroup();
    const auto has_new_asset_draft =
     !self.selected_asset_handle_opt_ && self.editing_asset_ptr_;
    const auto add_asset_requested =
     ImGui::Button(has_new_asset_draft ? PLUDUX_ICON_EDIT " Resume New Asset"
                                       : PLUDUX_ICON_ADD " New Asset");
    ImGui::SameLine();
    const auto load_assets_requested =
     ImGui::Button(PLUDUX_ICON_IMPORT " Import");
    ImGui::Spacing();
    ui::search_filter(self.asset_filter_, "##assets_search");
    ImGui::BeginChild("item view", ImVec2(0, 0));

    auto visible_asset_count = std::size_t{0};
    for(std::size_t i = 0; i < asset_handles.size(); ++i) {
      const auto asset_handle = asset_handles[i];
      const auto& asset = app_state.get_asset(asset_handle);
      if(!self.asset_filter_.PassFilter(asset.name().c_str())) {
        continue;
      }
      ++visible_asset_count;

      ImGui::PushID(i);

      {
        const auto selected =
         backtest_ptr && backtest_ptr->asset_handle() == asset_handle;
        const auto has_draft =
         self.selected_asset_handle_opt_ == asset_handle &&
         self.editing_asset_ptr_ && *self.editing_asset_ptr_ != asset;
        const auto display_name =
         has_draft ? asset.name() + " (Unsaved)" : asset.name();
        const auto action = ui::resource_row(
         display_name.c_str(), selected, i, asset_handles.size());
        if(action == ui::ResourceRowAction::Edit) {
          self.current_page_ = AssetPage::EditAsset;
          if(self.selected_asset_handle_opt_ != asset_handle ||
             !self.editing_asset_ptr_) {
            self.selected_asset_handle_opt_ = asset_handle;
            self.editing_asset_ptr_ = std::make_shared<backtest::Asset>(asset);
          }
        } else if(action == ui::ResourceRowAction::Duplicate) {
          context.push_action([asset_handle](ApplicationState& app_state) {
            const auto& value = app_state.get_asset(asset_handle);
            auto copy = value;
            copy.name(value.name() + " Copy");
            app_state.add_asset(std::move(copy));
          });
        } else if(action == ui::ResourceRowAction::MoveUp) {
          context.push_action([from = i](ApplicationState& app_state) {
            app_state.reorder_list_asset(from, from - 1);
          });
        } else if(action == ui::ResourceRowAction::MoveDown) {
          context.push_action([from = i](ApplicationState& app_state) {
            app_state.reorder_list_asset(from, from + 1);
          });
        } else if(action == ui::ResourceRowAction::Delete) {
          context.push_action([asset_handle](ApplicationState& app_state) {
            app_state.remove_asset(asset_handle);
          });
        }
        ImGui::PopID();
        continue;
      }
    }

    if(asset_handles.empty()) {
      ImGui::TextDisabled("No assets yet. Add or import one to get started.");
    } else if(visible_asset_count == 0) {
      ImGui::TextDisabled("No assets match this search.");
    }

    ImGui::EndChild();
    if(add_asset_requested) {
      self.current_page_ = AssetPage::AddNewAsset;
      if(self.selected_asset_handle_opt_ || !self.editing_asset_ptr_) {
        self.selected_asset_handle_opt_ = std::nullopt;
        self.editing_asset_ptr_ = std::make_shared<backtest::Asset>();
      }
    }

    if(load_assets_requested) {
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

      pludux_js_open_multiple_text_files(".csv", &callback, &context);

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
      self.leave_editor();
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

    const auto selected_asset_handle = self.selected_asset_handle_opt_.value();
    const auto& selected_asset =
     context.app_state().get_asset(selected_asset_handle);
    const auto same_asset = selected_asset == *self.editing_asset_ptr_;

    if(ImGui::Button("OK")) {
      self.submit_asset_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.leave_editor();
    }

    ImGui::SameLine();
    const auto draft_action =
     ui::apply_reset_button(self.selected_draft_action_, !same_asset);
    if(draft_action == ui::DraftAction::Apply) {
      self.submit_asset_changes(context);
    } else if(draft_action == ui::DraftAction::Reset) {
      self.editing_asset_ptr_ =
       std::make_shared<backtest::Asset>(selected_asset);
    }

    ImGui::EndGroup();
  }

  void edit_asset_form(this auto& self)
  {
    ui::form_section(
     "Asset Details",
     "Name this price history so it is easy to identify in a backtest. A "
     "blank name is saved as 'Unnamed'.");
    {
      auto asset_name = self.editing_asset_ptr_->name();
      ui::field_label("Name");
      ImGui::InputTextWithHint("##asset_name", "Unnamed", &asset_name);
      self.editing_asset_ptr_->name(asset_name);
    }

    ui::form_section(
     "CSV Column Mapping",
     "Enter the exact column headers that contain each OHLCV value. Header "
     "matching is case-sensitive.");
    auto field_resolver = self.editing_asset_ptr_->field_resolver();
    {
      auto open_field = field_resolver.open_field();
      ui::field_label("Open price");
      ImGui::InputTextWithHint("##open_field", "open", &open_field);
      field_resolver.open_field(open_field);
    }
    {
      auto high_field = field_resolver.high_field();
      ui::field_label("High price");
      ImGui::InputTextWithHint("##high_field", "high", &high_field);
      field_resolver.high_field(high_field);
    }
    {
      auto low_field = field_resolver.low_field();
      ui::field_label("Low price");
      ImGui::InputTextWithHint("##low_field", "low", &low_field);
      field_resolver.low_field(low_field);
    }
    {
      auto close_field = field_resolver.close_field();
      ui::field_label("Close price");
      ImGui::InputTextWithHint("##close_field", "close", &close_field);
      field_resolver.close_field(close_field);
    }
    {
      auto volume_field = field_resolver.volume_field();
      ui::field_label("Volume");
      ImGui::InputTextWithHint("##volume_field", "volume", &volume_field);
      field_resolver.volume_field(volume_field);
    }

    self.editing_asset_ptr_->field_resolver(field_resolver);

    {
      ui::form_section(
       "Price History",
       "Choose a CSV file to load or replace the asset's price history using "
       "the column mapping above.");
      if(ImGui::Button("Select CSV File")) {
#ifdef __EMSCRIPTEN__
        pludux_js_open_managed_text_file_input(
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
  }

  void submit_asset_changes(this auto& self, WindowContext& context)
  {
    context.push_action(
     [asset_handle_opt = self.selected_asset_handle_opt_,
      edit_asset_ptr = self.editing_asset_ptr_](ApplicationState& app_state) {
       if(edit_asset_ptr->name().empty()) {
         edit_asset_ptr->name("Unnamed");
       }

       if(!asset_handle_opt.has_value()) {
         app_state.add_asset(std::move(*edit_asset_ptr));
         return;
       }

       app_state.update_asset(asset_handle_opt.value(), *edit_asset_ptr);
     });
  }

  void reset(this auto& self)
  {
    self.current_page_ = AssetPage::List;
    self.selected_asset_handle_opt_ = std::nullopt;
    self.editing_asset_ptr_ = nullptr;
  }

  void leave_editor(this auto& self) noexcept
  {
    self.current_page_ = AssetPage::List;
  }
};

} // namespace pludux::apps

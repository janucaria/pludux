#include <iterator>

#include <imgui.h>

#include "../app_state.hpp"

#include "./assets_window.hpp"

namespace pludux::apps {

AssetsWindow::AssetsWindow()
{
}

void AssetsWindow::render(AppState& app_state)
{
  auto& state = app_state.state();
  auto& assets = state.assets;

  ImGui::Begin("Assets", nullptr);

  if(!assets.empty()) {
    for(auto i = 0; i < assets.size(); ++i) {
      const auto& asset = *assets[i];
      const auto& asset_name = asset.name();
      auto is_selected = state.selected_asset_index == i;

      ImGui::PushID(i);

      ImGui::SetNextItemAllowOverlap();
      if(ImGui::Selectable(asset_name.c_str(), &is_selected)) {
        app_state.push_action(
         [i](AppStateData& state) { state.selected_asset_index = i; });
      }
      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
      if(ImGui::Button("Delete")) {
        app_state.push_action([i](AppStateData& state) {
          auto old_backtests = std::move(state.backtests);
          state.backtests.clear();
          for(auto j = 0; j < old_backtests.size(); ++j) {
            if(j != i) {
              state.backtests.push_back(std::move(old_backtests[j]));
            }
          }
          state.assets.erase(std::next(state.assets.begin(), i));
          if(state.selected_asset_index >= i) {
            state.selected_asset_index--;
          }
        });
      }

      ImGui::PopID();
    }
  }

  ImGui::End();
}

} // namespace pludux::apps

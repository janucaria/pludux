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

      ImGui::PushID(i);

      ImGui::SetNextItemAllowOverlap();

      ImGui::Text("%s", asset_name.c_str());

      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
      if(ImGui::Button("Delete")) {
        app_state.push_action([i](AppStateData& state) {
          const auto& assets = state.assets;

          const auto it = std::next(assets.begin(), i);
          const auto& asset_ptr = *it;

          state.backtests.erase(std::remove_if(state.backtests.begin(),
                                               state.backtests.end(),
                                               [&asset_ptr](const auto& bt) {
                                                 return bt.asset_ptr() ==
                                                        asset_ptr;
                                               }),
                                state.backtests.end());

          // Remove the asset from the vector
          state.assets.erase(it);
        });
      }

      ImGui::PopID();
    }
  }

  ImGui::End();
}

} // namespace pludux::apps

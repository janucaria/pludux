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
    for(const auto& asset_ptr : assets) {
      const auto& asset_name = asset_ptr->name();

      ImGui::PushID(asset_name.c_str());

      ImGui::SetNextItemAllowOverlap();

      ImGui::Text("%s", asset_name.c_str());

      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
      if(ImGui::Button("Delete")) {
        app_state.push_action([asset_ptr](AppStateData& state) {
          const auto& assets = state.assets;
          auto& backtests = state.backtests;

          for(auto i = 0; i < backtests.size(); ++i) {
            auto& backtest = backtests[i];
            if(backtest.asset_ptr() == asset_ptr) {
              backtests.erase(std::next(backtests.begin(), i));

              if(state.selected_backtest_index > i || i == backtests.size()) {
                --state.selected_backtest_index;
              }

              // Adjust the index since we removed an element
              --i;
            }
          }

          // Remove the asset from the vector
          state.assets.erase(asset_ptr);
        });
      }

      ImGui::PopID();
    }
  }

  ImGui::End();
}

} // namespace pludux::apps

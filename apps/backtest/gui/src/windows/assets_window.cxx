module;

#include <iterator>

#include <imgui.h>

#include <pludux/asset_snapshot.hpp>

export module pludux.apps.backtest.windows:assets_window;

import pludux.apps.backtest.app_state;

export namespace pludux::apps {

class AssetsWindow {
public:
  void render(this const auto, AppState& app_state)
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

            auto& backtests = state.backtests;
            for(auto j = 0; j < backtests.size(); ++j) {
              auto& backtest = backtests[j];
              if(backtest.asset_ptr()->name() == asset_ptr->name()) {
                backtests.erase(std::next(backtests.begin(), j));

                if(state.selected_backtest_index > i ||
                   state.selected_backtest_index >= backtests.size()) {
                  --state.selected_backtest_index;
                }

                // Adjust the index since we removed an element
                --j;
              }
            }
            // Remove the asset from the vector
            state.assets.erase(it);
          });
        }

        ImGui::PopID();
      }
    }

    ImGui::End();
  }
};

} // namespace pludux::apps

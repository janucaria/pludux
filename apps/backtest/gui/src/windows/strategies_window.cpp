#include <iterator>

#include <imgui.h>

#include "../app_state.hpp"

#include "./strategies_window.hpp"

namespace pludux::apps {

StrategiesWindow::StrategiesWindow()
{
}

void StrategiesWindow::render(AppState& app_state)
{
  auto& state = app_state.state();
  auto& strategies = state.strategies;

  ImGui::Begin("Strategies", nullptr);

  if(!strategies.empty()) {
    for(auto i = 0; i < strategies.size(); ++i) {
      const auto& strategy = *strategies[i];
      const auto& strategy_name = strategy.name();
      auto is_selected = state.selected_strategy_index == i;

      ImGui::PushID(i);

      ImGui::SetNextItemAllowOverlap();

      ImGui::Text("%s", strategy_name.c_str());

      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);

      if(!is_selected) {
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 88);

        if(ImGui::Button("Use")) {
          app_state.push_action([i](AppStateData& state) {
            const auto& strategy = *state.strategies[i];
            state.selected_strategy_index = i;

            state.backtests.clear();
            for(const auto& asset_ptr : state.assets) {
              const auto& asset = *asset_ptr;
              state.backtests.emplace_back(strategy, asset, state.quote_access);
            }
          });
        }

        ImGui::SameLine();
      }

      if(ImGui::Button("Delete")) {
        app_state.push_action([i](AppStateData& state) {
          const auto old_selected_strategy_index =
           state.selected_strategy_index;

          state.strategies.erase(std::next(state.strategies.begin(), i));
          if(state.selected_strategy_index >= i) {
            state.selected_strategy_index--;
          }

          if(old_selected_strategy_index == i) {
            state.backtests.clear();

            if(!state.strategies.empty()) {
              const auto& strategy =
               *state.strategies[state.selected_strategy_index];
              for(const auto& asset_ptr : state.assets) {
                const auto& asset = *asset_ptr;
                state.backtests.emplace_back(
                 strategy, asset, state.quote_access);
              }
            } else {
              state.selected_strategy_index = -1;
            }
          }
        });
      }

      ImGui::PopID();
    }
  }

  ImGui::End();
}

} // namespace pludux::apps

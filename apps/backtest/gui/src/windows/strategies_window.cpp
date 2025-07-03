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

      ImGui::PushID(i);

      ImGui::SetNextItemAllowOverlap();

      ImGui::Text("%s", strategy_name.c_str());

      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);

      if(ImGui::Button("Delete")) {
        app_state.push_action([i](AppStateData& state) {
          const auto& strategies = state.strategies;
          const auto it = std::next(strategies.begin(), i);
          const auto& strategy_ptr = *it;

          auto& backtests = state.backtests;
          for(auto j = 0; j < backtests.size(); ++j) {
            auto& backtest = backtests[j];
            if(backtest.strategy_ptr()->name() == strategy_ptr->name()) {
              backtests.erase(std::next(backtests.begin(), j));

              if(state.selected_backtest_index > j ||
                 state.selected_backtest_index >= backtests.size()) {
                --state.selected_backtest_index;
              }

              // Adjust the index since we removed an element
              --j;
            }
          }

          state.strategies.erase(it);
        });
      }

      ImGui::PopID();
    }
  }

  ImGui::End();
}

} // namespace pludux::apps

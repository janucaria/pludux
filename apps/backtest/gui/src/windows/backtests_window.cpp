#include <format>
#include <iterator>

#include <imgui.h>

#include "../app_state.hpp"

#include "./backtests_window.hpp"

namespace pludux::apps {

BacktestsWindow::BacktestsWindow()
{
}

void BacktestsWindow::render(AppState& app_state)
{
  ImGui::Begin("Backtests", nullptr);

  if(is_adding_new_backtest_) {
    render_add_new_backtest(app_state);
  } else {
    render_backtests_list(app_state);
  }

  ImGui::End();
}

void BacktestsWindow::render_backtests_list(AppState& app_state)
{
  auto& state = app_state.state();
  auto& backtests = state.backtests;

  ImGui::BeginGroup();
  ImGui::BeginChild(
   "item view",
   ImVec2(
    0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

  if(!backtests.empty()) {
    for(auto i = 0; i < backtests.size(); ++i) {
      const auto& backtest = backtests[i];
      const auto& backtest_name = backtest.name();
      auto is_selected = state.selected_backtest_index == i;

      ImGui::PushID(i);

      ImGui::SetNextItemAllowOverlap();
      if(ImGui::Selectable(backtest_name.c_str(), &is_selected)) {
        app_state.push_action(
         [i](AppStateData& state) { state.selected_backtest_index = i; });
      }
      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
      if(ImGui::Button("Delete")) {
        app_state.push_action([i](AppStateData& state) {
          auto& backtests = state.backtests;

          backtests.erase(std::next(backtests.begin(), i));

          if(state.selected_backtest_index > i ||
             state.selected_backtest_index >= backtests.size()) {
            --state.selected_backtest_index;
          }
        });
      }

      ImGui::PopID();
    }
  }

  ImGui::EndChild();

  if(ImGui::Button("Add New Backtest")) {
    is_adding_new_backtest_ = true;

    new_backtest_name_ = "";
  }
  ImGui::EndGroup();
}

void BacktestsWindow::render_add_new_backtest(AppState& app_state)
{
  const auto& state = app_state.state();
  const auto& strategies = state.strategies;
  const auto& assets = state.assets;
  const auto& profiles = state.profiles;

  ImGui::BeginGroup();
  ImGui::BeginChild("item view",
                    ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

  ImGui::Text("Add New Backtest");
  ImGui::Separator();
  ImGui::SetNextItemWidth(-1); // Full width for input text

  {
    ImGui::Text("Name:");

    char name_buffer[64];
    std::strcpy(name_buffer, new_backtest_name_.c_str());
    ImGui::InputText("##NewBacktestName", name_buffer, sizeof(name_buffer));

    new_backtest_name_ = std::string{name_buffer};
  }

  {
    new_backtest_strategy_index_ =
     std::min(new_backtest_strategy_index_,
              static_cast<std::ptrdiff_t>(strategies.size() - 1));

    auto strategy_preview = std::string{""};
    if(new_backtest_strategy_index_ >= 0 &&
       new_backtest_strategy_index_ < strategies.size()) {
      strategy_preview = strategies[new_backtest_strategy_index_]->name();
    }

    ImGui::Text("Strategy:");
    if(ImGui::BeginCombo("##StrategyCombo", strategy_preview.c_str())) {
      for(auto i = 0; i < strategies.size(); ++i) {
        const auto& strategy = *strategies[i];
        const auto& strategy_name = strategy.name();
        const auto is_selected = new_backtest_strategy_index_ == i;

        ImGui::PushID(i);

        if(ImGui::Selectable(strategy_name.c_str(), is_selected)) {
          new_backtest_strategy_index_ = i;
        }

        if(is_selected) {
          ImGui::SetItemDefaultFocus();
        }

        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
  }

  {
    new_backtest_asset_index_ = std::min(
     new_backtest_asset_index_, static_cast<std::ptrdiff_t>(assets.size() - 1));

    auto asset_preview = std::string{""};
    if(new_backtest_asset_index_ >= 0 &&
       new_backtest_asset_index_ < assets.size()) {
      asset_preview = assets[new_backtest_asset_index_]->name();
    }

    ImGui::Text("Asset:");
    if(ImGui::BeginCombo("##AssetCombo", asset_preview.c_str())) {
      for(auto i = 0; i < assets.size(); ++i) {
        const auto& asset = *assets[i];
        const auto& asset_name = asset.name();
        const auto is_selected = new_backtest_asset_index_ == i;

        ImGui::PushID(i);

        if(ImGui::Selectable(asset_name.c_str(), is_selected)) {
          new_backtest_asset_index_ = i;
        }

        if(is_selected) {
          ImGui::SetItemDefaultFocus();
        }

        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
  }

  {
    new_backtest_profile_index_ =
     std::min(new_backtest_profile_index_,
              static_cast<std::ptrdiff_t>(state.profiles.size() - 1));

    auto profile_preview = std::string{""};
    if(new_backtest_profile_index_ >= 0 &&
       new_backtest_profile_index_ < profiles.size()) {
      profile_preview = profiles[new_backtest_profile_index_]->name();
    }

    ImGui::Text("Profile:");
    if(ImGui::BeginCombo("##ProfileCombo", profile_preview.c_str())) {
      for(auto i = 0; i < profiles.size(); ++i) {
        const auto& profile = *profiles[i];
        const auto& profile_name = profile.name();
        const auto is_selected = new_backtest_profile_index_ == i;

        ImGui::PushID(i);

        if(ImGui::Selectable(profile_name.c_str(), is_selected)) {
          new_backtest_profile_index_ = i;
        }

        if(is_selected) {
          ImGui::SetItemDefaultFocus();
        }

        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
  }

  ImGui::EndChild();

  if(ImGui::Button("Create Backtest")) {
    if(new_backtest_strategy_index_ >= 0 && new_backtest_asset_index_ >= 0 &&
       new_backtest_profile_index_ >= 0) {
      app_state.push_action(
       [new_backtest_name = new_backtest_name_,
        strategy_index = new_backtest_strategy_index_,
        asset_index = new_backtest_asset_index_,
        profile_index = new_backtest_profile_index_](AppStateData& state) {
         const auto& strategy = state.strategies[strategy_index];
         const auto& asset = state.assets[asset_index];
         const auto& profile = state.profiles[profile_index];

         const auto backtest_name =
          new_backtest_name.empty()
           ? std::format(
              "{} / {} / {}", asset->name(), strategy->name(), profile->name())
           : new_backtest_name;

         state.backtests.emplace_back(backtest_name, strategy, asset, profile);
         state.selected_backtest_index = state.backtests.size() - 1;
       });

      is_adding_new_backtest_ = false;
    } else {
      app_state.push_action([](AppStateData& state) {
        state.alert_messages.push(
         "Please select a strategy, an asset, and a profile.");
      });
    }
  }

  ImGui::SameLine();
  if(ImGui::Button("Cancel")) {
    is_adding_new_backtest_ = false;
  }

  ImGui::EndGroup();
}

} // namespace pludux::apps

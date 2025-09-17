module;

#include <format>
#include <iterator>

#include <imgui.h>

#include "../app_state_data.hpp"

export module pludux.apps.backtest.windows:backtests_window;

import pludux.apps.backtest.app_state;

export namespace pludux::apps {

class BacktestsWindow {
public:
  BacktestsWindow()
  : is_adding_new_backtest_{false}
  , new_backtest_name_{""}
  , new_backtest_strategy_index_{0}
  , new_backtest_asset_index_{0}
  , new_backtest_profile_index_{0}
  {
  }

  void render(this auto& self, AppState& app_state)
  {
    ImGui::Begin("Backtests", nullptr);

    if(self.is_adding_new_backtest_) {
      self.render_add_new_backtest(app_state);
    } else {
      self.render_backtests_list(app_state);
    }

    ImGui::End();
  }

private:
  bool is_adding_new_backtest_;

  std::string new_backtest_name_;
  std::ptrdiff_t new_backtest_strategy_index_;
  std::ptrdiff_t new_backtest_asset_index_;
  std::ptrdiff_t new_backtest_profile_index_;

  void render_backtests_list(this auto& self, AppState& app_state)
  {
    auto& state = app_state.state();
    auto& backtests = state.backtests;

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

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
      self.is_adding_new_backtest_ = true;
      self.new_backtest_name_ = "";
    }
    ImGui::EndGroup();
  }

  void render_add_new_backtest(this auto& self, AppState& app_state)
  {
    auto& new_backtest_strategy_index = self.new_backtest_strategy_index_;
    auto& new_backtest_asset_index = self.new_backtest_asset_index_;
    auto& new_backtest_profile_index = self.new_backtest_profile_index_;
    auto& new_backtest_name = self.new_backtest_name_;
    auto& is_adding_new_backtest = self.is_adding_new_backtest_;

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
      std::strcpy(name_buffer, new_backtest_name.c_str());
      ImGui::InputText("##NewBacktestName", name_buffer, sizeof(name_buffer));

      new_backtest_name = std::string{name_buffer};
    }

    {
      new_backtest_strategy_index =
       std::min(new_backtest_strategy_index,
                static_cast<std::ptrdiff_t>(strategies.size() - 1));

      auto strategy_preview = std::string{""};
      if(new_backtest_strategy_index >= 0 &&
         new_backtest_strategy_index < strategies.size()) {
        strategy_preview = strategies[new_backtest_strategy_index]->name();
      }

      ImGui::Text("Strategy:");
      if(ImGui::BeginCombo("##StrategyCombo", strategy_preview.c_str())) {
        for(auto i = 0; i < strategies.size(); ++i) {
          const auto& strategy = *strategies[i];
          const auto& strategy_name = strategy.name();
          const auto is_selected = new_backtest_strategy_index == i;

          ImGui::PushID(i);

          if(ImGui::Selectable(strategy_name.c_str(), is_selected)) {
            new_backtest_strategy_index = i;
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
      new_backtest_asset_index =
       std::min(new_backtest_asset_index,
                static_cast<std::ptrdiff_t>(assets.size() - 1));

      auto asset_preview = std::string{""};
      if(new_backtest_asset_index >= 0 &&
         new_backtest_asset_index < assets.size()) {
        asset_preview = assets[new_backtest_asset_index]->name();
      }

      ImGui::Text("Asset:");
      if(ImGui::BeginCombo("##AssetCombo", asset_preview.c_str())) {
        for(auto i = 0; i < assets.size(); ++i) {
          const auto& asset = *assets[i];
          const auto& asset_name = asset.name();
          const auto is_selected = new_backtest_asset_index == i;

          ImGui::PushID(i);

          if(ImGui::Selectable(asset_name.c_str(), is_selected)) {
            new_backtest_asset_index = i;
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
      new_backtest_profile_index =
       std::min(new_backtest_profile_index,
                static_cast<std::ptrdiff_t>(state.profiles.size() - 1));

      auto profile_preview = std::string{""};
      if(new_backtest_profile_index >= 0 &&
         new_backtest_profile_index < profiles.size()) {
        profile_preview = profiles[new_backtest_profile_index]->name();
      }

      ImGui::Text("Profile:");
      if(ImGui::BeginCombo("##ProfileCombo", profile_preview.c_str())) {
        for(auto i = 0; i < profiles.size(); ++i) {
          const auto& profile = *profiles[i];
          const auto& profile_name = profile.name();
          const auto is_selected = new_backtest_profile_index == i;

          ImGui::PushID(i);

          if(ImGui::Selectable(profile_name.c_str(), is_selected)) {
            new_backtest_profile_index = i;
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
      if(new_backtest_strategy_index >= 0 && new_backtest_asset_index >= 0 &&
         new_backtest_profile_index >= 0) {
        app_state.push_action([new_backtest_name,
                               strategy_index = new_backtest_strategy_index,
                               asset_index = new_backtest_asset_index,
                               profile_index = new_backtest_profile_index](
                               AppStateData& state) {
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

        is_adding_new_backtest = false;
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push(
           "Please select a strategy, an asset, and a profile.");
        });
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      is_adding_new_backtest = false;
    }

    ImGui::EndGroup();
  }
};

} // namespace pludux::apps

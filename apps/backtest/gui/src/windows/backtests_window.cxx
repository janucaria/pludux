module;

#include <algorithm>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.backtests_window;

import pludux.backtest;
import :app_state;

export namespace pludux::apps {

class BacktestsWindow {
public:
  BacktestsWindow()
  : backtest_panel_mode_{BacktestPanelMode::List}
  , old_backtest_{std::nullopt}
  , new_backtest_{nullptr}
  {
  }

  void render(this auto& self, AppState& app_state)
  {
    ImGui::Begin("Backtests", nullptr);

    switch(self.backtest_panel_mode_) {
    case BacktestPanelMode::List:
      self.render_backtests_list(app_state);
      break;
    case BacktestPanelMode::Edit:
      self.render_edit_backtest(app_state);
      break;
    case BacktestPanelMode::AddNew:
      self.render_add_new_backtest(app_state);
      break;
    }

    ImGui::End();
  }

private:
  enum class BacktestPanelMode { List, Edit, AddNew } backtest_panel_mode_;

  std::optional<std::reference_wrapper<const pludux::Backtest>> old_backtest_;
  std::shared_ptr<pludux::Backtest> new_backtest_;

  void render_backtests_list(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& backtests = state.backtests;

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!backtests.empty()) {
      for(auto i = 0; i < backtests.size(); ++i) {
        auto& backtest = backtests[i];
        const auto& backtest_name = backtest.name();
        auto is_selected = state.selected_backtest_index == i;

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();
        if(ImGui::Selectable(backtest_name.c_str(), &is_selected)) {
          app_state.push_action(
           [i](AppStateData& state) { state.selected_backtest_index = i; });
        }
        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        if(ImGui::Button("Edit")) {
          self.backtest_panel_mode_ = BacktestPanelMode::Edit;
          self.old_backtest_.emplace(std::cref(backtest));
          self.new_backtest_ = std::make_shared<pludux::Backtest>(backtest);
        }

        ImGui::SameLine();

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
      self.backtest_panel_mode_ = BacktestPanelMode::AddNew;
      self.new_backtest_ =
       std::make_shared<pludux::Backtest>("", nullptr, nullptr, nullptr);
    }
    ImGui::EndGroup();
  }

  void render_add_new_backtest(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& strategies = state.strategies;
    const auto& assets = state.assets;
    const auto& profiles = state.profiles;

    auto& new_backtest = self.new_backtest_;

    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Backtest");
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1); // Full width for input text

    {
      ImGui::Text("Name:");

      auto backtest_name = new_backtest->name();
      ImGui::InputText("##NewBacktestName", &backtest_name);
      new_backtest->name(std::move(backtest_name));
    }

    {
      if(new_backtest->asset_ptr() == nullptr && !assets.empty()) {
        new_backtest->asset_ptr(assets.front());
      }

      ImGui::Text("Asset:");
      auto asset_preview = new_backtest->asset_ptr()
                            ? new_backtest->asset_ptr()->name()
                            : std::string{""};
      if(ImGui::BeginCombo("##AssetCombo", asset_preview.c_str())) {
        for(auto i = 0; i < assets.size(); ++i) {
          const auto& asset = assets[i];
          const auto& asset_name = asset->name();
          const auto is_selected = new_backtest->asset_ptr() == asset;

          ImGui::PushID(i);

          if(ImGui::Selectable(asset_name.c_str(), is_selected)) {
            new_backtest->asset_ptr(asset);
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
      if(new_backtest->strategy_ptr() == nullptr && !strategies.empty()) {
        new_backtest->strategy_ptr(strategies.front());
      }

      ImGui::Text("Strategy:");
      auto strategy_preview = new_backtest->strategy_ptr()
                               ? new_backtest->strategy_ptr()->name()
                               : std::string{""};
      if(ImGui::BeginCombo("##StrategyCombo", strategy_preview.c_str())) {
        for(auto i = 0; i < strategies.size(); ++i) {
          const auto& strategy = strategies[i];
          const auto& strategy_name = strategy->name();
          const auto is_selected = new_backtest->strategy_ptr() == strategy;

          ImGui::PushID(i);

          if(ImGui::Selectable(strategy_name.c_str(), is_selected)) {
            new_backtest->strategy_ptr(strategy);
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
      if(new_backtest->profile_ptr() == nullptr && !profiles.empty()) {
        new_backtest->profile_ptr(profiles.front());
      }

      ImGui::Text("Profile:");
      auto profile_preview = new_backtest->profile_ptr()
                              ? new_backtest->profile_ptr()->name()
                              : std::string{""};
      if(ImGui::BeginCombo("##ProfileCombo", profile_preview.c_str())) {
        for(auto i = 0; i < profiles.size(); ++i) {
          const auto& profile = profiles[i];
          const auto& profile_name = profile->name();
          const auto is_selected = new_backtest->profile_ptr() == profile;

          ImGui::PushID(i);

          if(ImGui::Selectable(profile_name.c_str(), is_selected)) {
            new_backtest->profile_ptr(profile);
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
      if(new_backtest->strategy_ptr() && new_backtest->asset_ptr() &&
         new_backtest->profile_ptr()) {
        app_state.push_action([new_backtest](AppStateData& state) {
          const auto& strategy = new_backtest->strategy_ptr();
          const auto& asset = new_backtest->asset_ptr();
          const auto& profile = new_backtest->profile_ptr();
          const auto& new_backtest_name = new_backtest->name();

          const auto backtest_name =
           // TODO: Visual Studio 2026 have bug with include <format> causing
           // compile error
           new_backtest_name.empty()
            ? asset->name() + " / " + strategy->name() + " / " + profile->name()
            : new_backtest_name;
          new_backtest->name(backtest_name);

          state.backtests.emplace_back(std::move(*new_backtest));
          state.selected_backtest_index = state.backtests.size() - 1;
        });

        self.backtest_panel_mode_ = BacktestPanelMode::List;
        self.new_backtest_ = nullptr;
      } else {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push(
           "Please select a strategy, an asset, and a profile.");
        });
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.backtest_panel_mode_ = BacktestPanelMode::List;
    }

    ImGui::EndGroup();
  }

  void render_edit_backtest(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& strategies = state.strategies;
    const auto& assets = state.assets;
    const auto& profiles = state.profiles;

    auto& new_backtest = self.new_backtest_;

    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Backtest");
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1); // Full width for input text

    {
      ImGui::Text("Name:");

      auto backtest_name = new_backtest->name();
      ImGui::InputText("##NewBacktestName", &backtest_name);
      new_backtest->name(std::move(backtest_name));
    }

    {
      if(new_backtest->asset_ptr() == nullptr && !assets.empty()) {
        new_backtest->asset_ptr(assets.front());
      }

      ImGui::Text("Asset:");
      auto asset_preview = new_backtest->asset_ptr()
                            ? new_backtest->asset_ptr()->name()
                            : std::string{""};
      if(ImGui::BeginCombo("##AssetCombo", asset_preview.c_str())) {
        for(auto i = 0; i < assets.size(); ++i) {
          const auto& asset = assets[i];
          const auto& asset_name = asset->name();
          const auto is_selected = new_backtest->asset_ptr() == asset;

          ImGui::PushID(i);

          if(ImGui::Selectable(asset_name.c_str(), is_selected)) {
            new_backtest->asset_ptr(asset);
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
      if(new_backtest->strategy_ptr() == nullptr && !strategies.empty()) {
        new_backtest->strategy_ptr(strategies.front());
      }

      ImGui::Text("Strategy:");
      auto strategy_preview = new_backtest->strategy_ptr()
                               ? new_backtest->strategy_ptr()->name()
                               : std::string{""};
      if(ImGui::BeginCombo("##StrategyCombo", strategy_preview.c_str())) {
        for(auto i = 0; i < strategies.size(); ++i) {
          const auto& strategy = strategies[i];
          const auto& strategy_name = strategy->name();
          const auto is_selected = new_backtest->strategy_ptr() == strategy;

          ImGui::PushID(i);

          if(ImGui::Selectable(strategy_name.c_str(), is_selected)) {
            new_backtest->strategy_ptr(strategy);
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
      if(new_backtest->profile_ptr() == nullptr && !profiles.empty()) {
        new_backtest->profile_ptr(profiles.front());
      }

      ImGui::Text("Profile:");
      auto profile_preview = new_backtest->profile_ptr()
                              ? new_backtest->profile_ptr()->name()
                              : std::string{""};
      if(ImGui::BeginCombo("##ProfileCombo", profile_preview.c_str())) {
        for(auto i = 0; i < profiles.size(); ++i) {
          const auto& profile = profiles[i];
          const auto& profile_name = profile->name();
          const auto is_selected = new_backtest->profile_ptr() == profile;

          ImGui::PushID(i);

          if(ImGui::Selectable(profile_name.c_str(), is_selected)) {
            new_backtest->profile_ptr(profile);
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

    if(ImGui::Button("Edit")) {
      if(new_backtest->name().empty()) {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push("Backtest name cannot be empty.");
        });
      } else if(!new_backtest->strategy_ptr() || !new_backtest->asset_ptr() ||
                !new_backtest->profile_ptr()) {
        app_state.push_action([](AppStateData& state) {
          state.alert_messages.push(
           "Please select an asset, a strategy, and a profile.");
        });
      } else {
        app_state.push_action([&old_backtest = self.old_backtest_.value().get(),
                               new_backtest](AppStateData& state) {
          new_backtest->reset();

          std::ranges::replace_if(
           state.backtests,
           [&old_backtest](const pludux::Backtest& backtest) {
             return &backtest == &old_backtest;
           },
           std::move(*new_backtest));
        });

        self.backtest_panel_mode_ = BacktestPanelMode::List;
        self.new_backtest_ = nullptr;
        self.old_backtest_ = std::nullopt;
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.backtest_panel_mode_ = BacktestPanelMode::List;
      self.new_backtest_ = nullptr;
      self.old_backtest_ = std::nullopt;
    }

    ImGui::EndGroup();
  }
};

} // namespace pludux::apps

module;

#include <algorithm>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.backtests_window;

import pludux.backtest;
import :window_context;

export namespace pludux::apps {

class BacktestsWindow {
public:
  BacktestsWindow()
  : backtest_panel_mode_{BacktestPanelMode::List}
  , selected_backtest_ptr_{nullptr}
  , editing_backtest_ptr_{nullptr}
  {
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Backtests", nullptr);

    switch(self.backtest_panel_mode_) {
    case BacktestPanelMode::List:
      self.render_backtests_list(context);
      break;
    case BacktestPanelMode::Edit:
      self.render_edit_backtest(context);
      break;
    case BacktestPanelMode::AddNew:
      self.render_add_new_backtest(context);
      break;
    }

    ImGui::End();
  }

private:
  enum class BacktestPanelMode { List, Edit, AddNew } backtest_panel_mode_;

  std::shared_ptr<backtest::Backtest> selected_backtest_ptr_;
  std::shared_ptr<backtest::Backtest> editing_backtest_ptr_;

  void render_backtests_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& backtests = context.backtests();

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!backtests.empty()) {
      for(auto i = 0; i < backtests.size(); ++i) {
        auto backtest = backtests[i];
        const auto& backtest_name = backtest->name();
        auto is_selected = app_state.selected_backtest() == backtest;

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();
        if(ImGui::Selectable(backtest_name.c_str(), &is_selected)) {
          context.push_action([backtest](ApplicationState& app_state) {
            app_state.select_backtest(backtest);
          });
        }
        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        if(ImGui::Button("Edit")) {
          self.backtest_panel_mode_ = BacktestPanelMode::Edit;
          self.selected_backtest_ptr_ = backtest;
          self.editing_backtest_ptr_ =
           std::make_shared<backtest::Backtest>(*backtest);
        }

        ImGui::SameLine();

        if(ImGui::Button("Delete")) {
          context.push_action([i](ApplicationState& app_state) {
            app_state.remove_backtest_at_index(i);
          });
        }

        ImGui::PopID();
      }
    }

    ImGui::EndChild();

    if(ImGui::Button("Add New Backtest")) {
      self.backtest_panel_mode_ = BacktestPanelMode::AddNew;
      self.selected_backtest_ptr_ = nullptr;
      self.editing_backtest_ptr_ = std::make_shared<backtest::Backtest>();
    }
    ImGui::EndGroup();
  }

  void render_add_new_backtest(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Backtest");
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1); // Full width for input text

    self.edit_backtest_form(context);

    ImGui::EndChild();

    if(ImGui::Button("Create Backtest")) {
      self.submit_backtest_changes(context, true);
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::EndGroup();
  }

  void render_edit_backtest(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Backtest");
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1); // Full width for input text

    self.edit_backtest_form(context);

    ImGui::EndChild();

    const auto same_backtest =
     self.selected_backtest_ptr_->equal_rules_and_metadata(
      *self.editing_backtest_ptr_);

    if(ImGui::Button("OK")) {
      self.submit_backtest_changes(context, true);
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::SameLine();
    ImGui::BeginDisabled(same_backtest);
    if(ImGui::Button("Apply")) {
      self.submit_backtest_changes(context, false);
    }
    ImGui::EndDisabled();

    ImGui::EndGroup();
  }

  void edit_backtest_form(this BacktestsWindow& self, WindowContext& context)
  {
    const auto& assets = context.assets();
    const auto& strategies = context.strategies();
    const auto& markets = context.markets();
    const auto& brokers = context.brokers();
    const auto& profiles = context.profiles();

    auto& edit_backtest_ptr = self.editing_backtest_ptr_;
    {
      ImGui::Text("Name:");

      auto backtest_name = edit_backtest_ptr->name();
      ImGui::InputText("##NewBacktestName", &backtest_name);
      edit_backtest_ptr->name(std::move(backtest_name));
    }

    {
      if(!std::ranges::contains(assets, edit_backtest_ptr->asset_ptr())) {
        edit_backtest_ptr->asset_ptr(nullptr);
      }

      if(edit_backtest_ptr->asset_ptr() == nullptr && !assets.empty()) {
        edit_backtest_ptr->asset_ptr(assets.front());
      }

      ImGui::Text("Asset:");
      auto asset_preview = edit_backtest_ptr->asset_ptr()
                            ? edit_backtest_ptr->asset_ptr()->name()
                            : std::string{""};
      if(ImGui::BeginCombo("##AssetCombo", asset_preview.c_str())) {
        for(auto i = 0; i < assets.size(); ++i) {
          const auto& asset = assets[i];
          const auto& asset_name = asset->name();
          const auto is_selected = edit_backtest_ptr->asset_ptr() == asset;

          ImGui::PushID(i);

          if(ImGui::Selectable(asset_name.c_str(), is_selected)) {
            edit_backtest_ptr->asset_ptr(asset);
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
      if(!std::ranges::contains(strategies,
                                edit_backtest_ptr->strategy_ptr())) {
        edit_backtest_ptr->strategy_ptr(nullptr);
      }

      if(edit_backtest_ptr->strategy_ptr() == nullptr && !strategies.empty()) {
        edit_backtest_ptr->strategy_ptr(strategies.front());
      }

      ImGui::Text("Strategy:");
      auto strategy_preview = edit_backtest_ptr->strategy_ptr()
                               ? edit_backtest_ptr->strategy_ptr()->name()
                               : std::string{""};
      if(ImGui::BeginCombo("##StrategyCombo", strategy_preview.c_str())) {
        for(auto i = 0; i < strategies.size(); ++i) {
          const auto& strategy = strategies[i];
          const auto& strategy_name = strategy->name();
          const auto is_selected =
           edit_backtest_ptr->strategy_ptr() == strategy;

          ImGui::PushID(i);

          if(ImGui::Selectable(strategy_name.c_str(), is_selected)) {
            edit_backtest_ptr->strategy_ptr(strategy);
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
      if(!std::ranges::contains(markets, edit_backtest_ptr->market_ptr())) {
        edit_backtest_ptr->market_ptr(nullptr);
      }

      if(edit_backtest_ptr->market_ptr() == nullptr && !markets.empty()) {
        edit_backtest_ptr->market_ptr(markets.front());
      }

      ImGui::Text("Market:");
      auto market_preview = edit_backtest_ptr->market_ptr()
                             ? edit_backtest_ptr->market_ptr()->name()
                             : std::string{""};
      if(ImGui::BeginCombo("##MarketCombo", market_preview.c_str())) {
        for(auto i = 0; i < markets.size(); ++i) {
          const auto& market = markets[i];
          const auto& market_name = market->name();
          const auto is_selected = edit_backtest_ptr->market_ptr() == market;

          ImGui::PushID(i);

          if(ImGui::Selectable(market_name.c_str(), is_selected)) {
            edit_backtest_ptr->market_ptr(market);
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
      if(!std::ranges::contains(brokers, edit_backtest_ptr->broker_ptr())) {
        edit_backtest_ptr->broker_ptr(nullptr);
      }

      if(edit_backtest_ptr->broker_ptr() == nullptr && !brokers.empty()) {
        edit_backtest_ptr->broker_ptr(brokers.front());
      }

      ImGui::Text("Broker:");
      auto broker_preview = edit_backtest_ptr->broker_ptr()
                             ? edit_backtest_ptr->broker_ptr()->name()
                             : std::string{""};
      if(ImGui::BeginCombo("##BrokerCombo", broker_preview.c_str())) {
        for(auto i = 0; i < brokers.size(); ++i) {
          const auto& broker = brokers[i];
          const auto& broker_name = broker->name();
          const auto is_selected = edit_backtest_ptr->broker_ptr() == broker;

          ImGui::PushID(i);

          if(ImGui::Selectable(broker_name.c_str(), is_selected)) {
            edit_backtest_ptr->broker_ptr(broker);
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
      if(!std::ranges::contains(profiles, edit_backtest_ptr->profile_ptr())) {
        edit_backtest_ptr->profile_ptr(nullptr);
      }

      if(edit_backtest_ptr->profile_ptr() == nullptr && !profiles.empty()) {
        edit_backtest_ptr->profile_ptr(profiles.front());
      }

      ImGui::Text("Profile:");
      auto profile_preview = edit_backtest_ptr->profile_ptr()
                              ? edit_backtest_ptr->profile_ptr()->name()
                              : std::string{""};
      if(ImGui::BeginCombo("##ProfileCombo", profile_preview.c_str())) {
        for(auto i = 0; i < profiles.size(); ++i) {
          const auto& profile = profiles[i];
          const auto& profile_name = profile->name();
          const auto is_selected = edit_backtest_ptr->profile_ptr() == profile;

          ImGui::PushID(i);

          if(ImGui::Selectable(profile_name.c_str(), is_selected)) {
            edit_backtest_ptr->profile_ptr(profile);
          }

          if(is_selected) {
            ImGui::SetItemDefaultFocus();
          }

          ImGui::PopID();
        }
        ImGui::EndCombo();
      }
    }
  }

  void submit_backtest_changes(this auto& self,
                               WindowContext& context,
                               bool reset_on_success)
  {
    auto& edit_backtest_ptr = self.editing_backtest_ptr_;
    if(edit_backtest_ptr->is_valid_rules()) {
      context.push_action([backtest_ptr = self.selected_backtest_ptr_,
                           edit_backtest_ptr](ApplicationState& app_state) {
        if(edit_backtest_ptr->name().empty()) {
          edit_backtest_ptr->name("Unnamed");
        }

        if(backtest_ptr == nullptr) {
          app_state.add_backtest(edit_backtest_ptr);
          app_state.select_backtest(edit_backtest_ptr);
        } else {
          const auto rule_changed =
           !backtest_ptr->equal_rules(*edit_backtest_ptr);
          if(rule_changed) {
            edit_backtest_ptr->reset();
            *backtest_ptr = *edit_backtest_ptr;
          } else {
            backtest_ptr->name(edit_backtest_ptr->name());
          }
        }
      });

      if(reset_on_success) {
        self.reset();
      }
    } else {
      context.push_action([](ApplicationState& app_state) {
        app_state.alert("Please select an asset, a strategy, a "
                        "market, a broker, and a profile.");
      });
    }
  }

  void reset(this BacktestsWindow& self) noexcept
  {
    self.backtest_panel_mode_ = BacktestPanelMode::List;
    self.selected_backtest_ptr_ = nullptr;
    self.editing_backtest_ptr_ = nullptr;
  }
};

} // namespace pludux::apps

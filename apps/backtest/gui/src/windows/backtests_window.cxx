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
  , old_backtest_{nullptr}
  , new_backtest_{nullptr}
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

  std::shared_ptr<pludux::Backtest> old_backtest_;
  std::shared_ptr<pludux::Backtest> new_backtest_;

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
          self.old_backtest_ = backtest;
          self.new_backtest_ = std::make_shared<pludux::Backtest>(*backtest);
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
      self.new_backtest_ = std::make_shared<pludux::Backtest>(
       "", nullptr, nullptr, nullptr, nullptr, nullptr);
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
      auto& new_backtest = self.new_backtest_;
      if(new_backtest->asset_ptr() && new_backtest->strategy_ptr() &&
         new_backtest->market_ptr() && new_backtest->broker_ptr() &&
         new_backtest->profile_ptr()) {
        context.push_action([new_backtest](ApplicationState& app_state) {
          const auto& asset = new_backtest->asset_ptr();
          const auto& strategy = new_backtest->strategy_ptr();
          const auto& market = new_backtest->market_ptr();
          const auto& broker = new_backtest->broker_ptr();
          const auto& profile = new_backtest->profile_ptr();
          const auto& new_backtest_name = new_backtest->name();

          const auto backtest_name =
           // TODO: Visual Studio 2026 have bug with include <format> causing
           // compile error
           new_backtest_name.empty()
            ? strategy->name() + " / " + asset->name() + " / " +
               market->name() + " / " + broker->name() + " / " + profile->name()
            : new_backtest_name;
          new_backtest->name(backtest_name);

          app_state.add_backtest(new_backtest);
          app_state.select_backtest(new_backtest);
        });

        self.backtest_panel_mode_ = BacktestPanelMode::List;
        self.new_backtest_ = nullptr;
      } else {
        context.push_action([](ApplicationState& app_state) {
          app_state.alert("Please select an asset, a strategy, a "
                          "market, a broker, and a profile.");
        });
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.backtest_panel_mode_ = BacktestPanelMode::List;
    }

    ImGui::EndGroup();
  }

  void render_edit_backtest(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Backtest");
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1); // Full width for input text

    self.edit_backtest_form(context);

    ImGui::EndChild();

    if(ImGui::Button("Edit")) {
      auto& new_backtest = self.new_backtest_;
      if(new_backtest->name().empty()) {
        context.push_action([](ApplicationState& app_state) {
          app_state.alert("Backtest name cannot be empty.");
        });
      } else if(!new_backtest->asset_ptr() || !new_backtest->strategy_ptr() ||
                !new_backtest->market_ptr() || !new_backtest->broker_ptr() ||
                !new_backtest->profile_ptr()) {
        context.push_action([](ApplicationState& app_state) {
          app_state.alert("Please select an asset, a strategy, a "
                          "market, a broker, and a profile.");
        });
      } else {
        context.push_action([old_backtest = self.old_backtest_,
                             new_backtest](ApplicationState& app_state) {
          new_backtest->reset();
          app_state.replace_backtest(old_backtest, std::move(new_backtest));
        });

        self.backtest_panel_mode_ = BacktestPanelMode::List;
        self.new_backtest_ = nullptr;
        self.old_backtest_ = nullptr;
      }
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.backtest_panel_mode_ = BacktestPanelMode::List;
      self.new_backtest_ = nullptr;
      self.old_backtest_ = nullptr;
    }

    ImGui::EndGroup();
  }

  void edit_backtest_form(this BacktestsWindow& self, WindowContext& context)
  {
    const auto& assets = context.assets();
    const auto& strategies = context.strategies();
    const auto& markets = context.markets();
    const auto& brokers = context.brokers();
    const auto& profiles = context.profiles();

    auto& new_backtest = self.new_backtest_;
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
      if(new_backtest->market_ptr() == nullptr && !markets.empty()) {
        new_backtest->market_ptr(markets.front());
      }

      ImGui::Text("Market:");
      auto market_preview = new_backtest->market_ptr()
                             ? new_backtest->market_ptr()->name()
                             : std::string{""};
      if(ImGui::BeginCombo("##MarketCombo", market_preview.c_str())) {
        for(auto i = 0; i < markets.size(); ++i) {
          const auto& market = markets[i];
          const auto& market_name = market->name();
          const auto is_selected = new_backtest->market_ptr() == market;

          ImGui::PushID(i);

          if(ImGui::Selectable(market_name.c_str(), is_selected)) {
            new_backtest->market_ptr(market);
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
      if(new_backtest->broker_ptr() == nullptr && !brokers.empty()) {
        new_backtest->broker_ptr(brokers.front());
      }

      ImGui::Text("Broker:");
      auto broker_preview = new_backtest->broker_ptr()
                             ? new_backtest->broker_ptr()->name()
                             : std::string{""};
      if(ImGui::BeginCombo("##BrokerCombo", broker_preview.c_str())) {
        for(auto i = 0; i < brokers.size(); ++i) {
          const auto& broker = brokers[i];
          const auto& broker_name = broker->name();
          const auto is_selected = new_backtest->broker_ptr() == broker;

          ImGui::PushID(i);

          if(ImGui::Selectable(broker_name.c_str(), is_selected)) {
            new_backtest->broker_ptr(broker);
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
  }
};

} // namespace pludux::apps

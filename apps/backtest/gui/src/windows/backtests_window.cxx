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
  , selected_backtest_handle_opt_{std::nullopt}
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

  std::optional<backtest::BacktestStoreHandle>
   selected_backtest_handle_opt_;
  std::shared_ptr<backtest::Backtest> editing_backtest_ptr_;

  void render_backtests_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& backtest_handles = app_state.get_backtest_handles();

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!backtest_handles.empty()) {
      for(auto i = 0; i < backtest_handles.size(); ++i) {
        const auto backtest_handle = backtest_handles[i];
        const auto& backtest = app_state.get_backtest(backtest_handle);
        const auto& backtest_name = backtest.name();
        auto is_selected =
         app_state.selected_backtest_handle() == backtest_handle;

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();
        if(ImGui::Selectable(backtest_name.c_str(), &is_selected)) {
          context.push_action([backtest_handle](ApplicationState& app_state) {
            app_state.select_backtest(backtest_handle);
          });
        }
        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        if(ImGui::Button("Edit")) {
          self.backtest_panel_mode_ = BacktestPanelMode::Edit;
          self.selected_backtest_handle_opt_ = backtest_handle;
          self.editing_backtest_ptr_ =
           std::make_shared<backtest::Backtest>(backtest);
        }

        ImGui::SameLine();

        if(ImGui::Button("Delete")) {
          context.push_action([backtest_handle](ApplicationState& app_state) {
            app_state.remove_backtest(backtest_handle);
          });
        }

        ImGui::PopID();
      }
    }

    ImGui::EndChild();

    if(ImGui::Button("Add New Backtest")) {
      self.backtest_panel_mode_ = BacktestPanelMode::AddNew;
      self.selected_backtest_handle_opt_ = std::nullopt;
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

    const auto selected_backtest_handle =
     self.selected_backtest_handle_opt_.value();
    const auto& selected_backtest =
     context.app_state().get_backtest(selected_backtest_handle);
    const auto same_backtest = selected_backtest.equivalent_rules_and_metadata(
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
    const auto& app_state = context.app_state();

    auto& edit_backtest_ptr = self.editing_backtest_ptr_;
    {
      ImGui::Text("Name:");

      auto backtest_name = edit_backtest_ptr->name();
      ImGui::InputText("##NewBacktestName", &backtest_name);
      edit_backtest_ptr->name(std::move(backtest_name));
    }

    {
      ImGui::Text("Initial Capital:");

      auto initial_capital = edit_backtest_ptr->initial_capital();
      ImGui::InputDouble(
       "##NewInitialCapital", &initial_capital, 100.0, 1000.0, "%.0f");
      edit_backtest_ptr->initial_capital(initial_capital);
    }

    {
      const auto& asset_handles = app_state.get_asset_handles();
      const auto edit_asset_handle = edit_backtest_ptr->asset_handle();
      const auto& edit_asset_ptr =
       app_state.get_asset_if_present(edit_asset_handle);

      if(!edit_asset_ptr) {
        if(!self.selected_backtest_handle_opt_ && !asset_handles.empty()) {
          edit_backtest_ptr->asset_handle(asset_handles.front());
        }
      }

      ImGui::Text("Asset:");
      auto asset_preview =
       edit_asset_ptr ? edit_asset_ptr->name() : std::string{""};
      if(ImGui::BeginCombo("##AssetCombo", asset_preview.c_str())) {
        for(auto i = 0; i < asset_handles.size(); ++i) {
          const auto& asset_handle = asset_handles[i];
          const auto& asset = app_state.get_asset(asset_handle);
          const auto& asset_name = asset.name();
          const auto is_selected =
           edit_backtest_ptr->asset_handle() == asset_handle;

          ImGui::PushID(i);

          if(ImGui::Selectable(asset_name.c_str(), is_selected)) {
            edit_backtest_ptr->asset_handle(asset_handle);
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
      const auto& strategy_handles = app_state.get_strategy_handles();
      const auto edit_strategy_handle = edit_backtest_ptr->strategy_handle();
      const auto& edit_strategy_ptr =
       app_state.get_strategy_if_present(edit_strategy_handle);

      if(!edit_strategy_ptr) {
        if(!self.selected_backtest_handle_opt_ && !strategy_handles.empty()) {
          edit_backtest_ptr->strategy_handle(strategy_handles.front());
        }
      }

      ImGui::Text("Strategy:");
      auto strategy_preview =
       edit_strategy_ptr ? edit_strategy_ptr->name() : std::string{""};
      if(ImGui::BeginCombo("##StrategyCombo", strategy_preview.c_str())) {
        for(auto i = 0; i < strategy_handles.size(); ++i) {
          const auto& strategy_handle = strategy_handles[i];
          const auto& strategy = app_state.get_strategy(strategy_handle);
          const auto& strategy_name = strategy.name();
          const auto is_selected =
           edit_backtest_ptr->strategy_handle() == strategy_handle;

          ImGui::PushID(i);

          if(ImGui::Selectable(strategy_name.c_str(), is_selected)) {
            edit_backtest_ptr->strategy_handle(strategy_handle);
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
      const auto& market_handles = app_state.get_market_handles();
      const auto edit_market_handle = edit_backtest_ptr->market_handle();
      const auto& edit_market_ptr =
       app_state.get_market_if_present(edit_market_handle);

      if(!edit_market_ptr) {
        if(!self.selected_backtest_handle_opt_ && !market_handles.empty()) {
          edit_backtest_ptr->market_handle(market_handles.front());
        }
      }

      ImGui::Text("Market:");
      auto market_preview =
       edit_market_ptr ? edit_market_ptr->name() : std::string{""};
      if(ImGui::BeginCombo("##MarketCombo", market_preview.c_str())) {
        for(auto i = 0; i < market_handles.size(); ++i) {
          const auto& market_handle = market_handles[i];
          const auto& market = app_state.get_market(market_handle);
          const auto& market_name = market.name();
          const auto is_selected =
           edit_backtest_ptr->market_handle() == market_handle;

          ImGui::PushID(i);

          if(ImGui::Selectable(market_name.c_str(), is_selected)) {
            edit_backtest_ptr->market_handle(market_handle);
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
      const auto& broker_handles = app_state.get_broker_handles();
      const auto edit_broker_handle = edit_backtest_ptr->broker_handle();
      const auto& edit_broker_ptr =
       app_state.get_broker_if_present(edit_broker_handle);

      if(!edit_broker_ptr) {
        if(!self.selected_backtest_handle_opt_ && !broker_handles.empty()) {
          edit_backtest_ptr->broker_handle(broker_handles.front());
        }
      }

      ImGui::Text("Broker:");
      auto broker_preview =
       edit_broker_ptr ? edit_broker_ptr->name() : std::string{""};
      if(ImGui::BeginCombo("##BrokerCombo", broker_preview.c_str())) {
        for(auto i = 0; i < broker_handles.size(); ++i) {
          const auto& broker_handle = broker_handles[i];
          const auto& broker = app_state.get_broker(broker_handle);
          const auto& broker_name = broker.name();
          const auto is_selected =
           edit_backtest_ptr->broker_handle() == broker_handle;

          ImGui::PushID(i);

          if(ImGui::Selectable(broker_name.c_str(), is_selected)) {
            edit_backtest_ptr->broker_handle(broker_handle);
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
      const auto& profile_handles = app_state.get_profile_handles();
      const auto edit_profile_handle = edit_backtest_ptr->profile_handle();
      const auto& edit_profile_ptr =
       app_state.get_profile_if_present(edit_profile_handle);

      if(!edit_profile_ptr) {
        if(!self.selected_backtest_handle_opt_ && !profile_handles.empty()) {
          edit_backtest_ptr->profile_handle(profile_handles.front());
        }
      }

      ImGui::Text("Profile:");
      auto profile_preview =
       edit_profile_ptr ? edit_profile_ptr->name() : std::string{""};
      if(ImGui::BeginCombo("##ProfileCombo", profile_preview.c_str())) {
        for(auto i = 0; i < profile_handles.size(); ++i) {
          const auto profile_handle = profile_handles[i];
          const auto& profile = app_state.get_profile(profile_handle);
          const auto& profile_name = profile.name();
          const auto is_selected =
           edit_backtest_ptr->profile_handle() == profile_handle;

          ImGui::PushID(i);

          if(ImGui::Selectable(profile_name.c_str(), is_selected)) {
            edit_backtest_ptr->profile_handle(profile_handle);
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
    if(context.app_state().is_backtest_ready(*edit_backtest_ptr)) {
      context.push_action([backtest_handle_opt =
                            self.selected_backtest_handle_opt_,
                           edit_backtest_ptr](ApplicationState& app_state) {
        if(edit_backtest_ptr->name().empty()) {
          edit_backtest_ptr->name("Unnamed");
        }

        if(!backtest_handle_opt) {
          const auto edit_handle_opt =
           app_state.add_backtest(*edit_backtest_ptr);
          app_state.select_backtest(edit_handle_opt.value());
        } else {
          const auto backtest_handle = backtest_handle_opt.value();
          auto& backtest = app_state.get_backtest(backtest_handle);
          const auto rule_changed =
           !backtest.equivalent_rules(*edit_backtest_ptr);
          if(rule_changed) {
            app_state.update_backtest(*backtest_handle_opt, *edit_backtest_ptr);
          } else {
            backtest.name(edit_backtest_ptr->name());
          }
        }
      });

      if(reset_on_success) {
        self.reset();
      }
    } else {
      context.alert("Please select an asset, a strategy, a "
                    "market, a broker, and a profile.");
    }
  }

  void reset(this BacktestsWindow& self) noexcept
  {
    self.backtest_panel_mode_ = BacktestPanelMode::List;
    self.selected_backtest_handle_opt_ = std::nullopt;
    self.editing_backtest_ptr_ = nullptr;
  }
};

} // namespace pludux::apps

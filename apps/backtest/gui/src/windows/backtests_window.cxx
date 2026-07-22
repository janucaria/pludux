module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.backtests_window;

import pludux.backtest;
import :window_context;
import :ui.widgets;

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
    ImGui::Begin("Backtests");

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

  void discard_draft(this BacktestsWindow& self) noexcept
  {
    self.reset();
  }

private:
  enum class BacktestPanelMode { List, Edit, AddNew } backtest_panel_mode_;

  std::optional<backtest::BacktestStoreHandle> selected_backtest_handle_opt_;
  std::shared_ptr<backtest::Backtest> editing_backtest_ptr_;
  ImGuiTextFilter backtest_filter_;
  ui::DraftAction selected_draft_action_{ui::DraftAction::Apply};

  void render_backtests_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& backtest_handles = app_state.get_backtest_handles();

    ImGui::BeginGroup();
    const auto has_new_backtest_draft =
     !self.selected_backtest_handle_opt_ && self.editing_backtest_ptr_;
    if(ImGui::Button(has_new_backtest_draft
                      ? PLUDUX_ICON_EDIT " Resume New Backtest"
                      : PLUDUX_ICON_ADD " New Backtest")) {
      self.backtest_panel_mode_ = BacktestPanelMode::AddNew;
      if(self.selected_backtest_handle_opt_ || !self.editing_backtest_ptr_) {
        self.selected_backtest_handle_opt_ = std::nullopt;
        self.editing_backtest_ptr_ = std::make_shared<backtest::Backtest>();
      }
    }
    ImGui::Spacing();
    ui::search_filter(self.backtest_filter_, "##backtests_search");
    ImGui::BeginChild("item view", ImVec2(0, 0));

    auto visible_backtest_count = std::size_t{0};
    if(!backtest_handles.empty()) {
      for(std::size_t i = 0; i < backtest_handles.size(); ++i) {
        const auto backtest_handle = backtest_handles[i];
        const auto& backtest = app_state.get_backtest(backtest_handle);
        const auto& backtest_name = backtest.name();
        if(!self.backtest_filter_.PassFilter(backtest_name.c_str())) {
          continue;
        }
        ++visible_backtest_count;
        auto is_selected =
         app_state.selected_backtest_handle() == backtest_handle;

        ImGui::PushID(i);

        {
          const auto has_draft =
           self.selected_backtest_handle_opt_ == backtest_handle &&
           self.editing_backtest_ptr_ &&
           *self.editing_backtest_ptr_ != backtest;
          const auto display_name =
           has_draft ? backtest_name + " (Unsaved)" : backtest_name;
          const auto action = ui::resource_row(
           display_name.c_str(), is_selected, i, backtest_handles.size());
          if(action == ui::ResourceRowAction::Select) {
            context.push_action([backtest_handle](ApplicationState& app_state) {
              app_state.select_backtest(backtest_handle);
            });
          } else if(action == ui::ResourceRowAction::Edit) {
            self.backtest_panel_mode_ = BacktestPanelMode::Edit;
            if(self.selected_backtest_handle_opt_ != backtest_handle ||
               !self.editing_backtest_ptr_) {
              self.selected_backtest_handle_opt_ = backtest_handle;
              self.editing_backtest_ptr_ =
               std::make_shared<backtest::Backtest>(backtest);
            }
          } else if(action == ui::ResourceRowAction::Duplicate) {
            context.push_action([backtest_handle](ApplicationState& app_state) {
              const auto& value = app_state.get_backtest(backtest_handle);
              auto copy = value;
              copy.name(value.name() + " Copy");
              app_state.add_backtest(std::move(copy));
            });
          } else if(action == ui::ResourceRowAction::MoveUp) {
            context.push_action([from = i](ApplicationState& app_state) {
              app_state.reorder_list_backtest(from, from - 1);
            });
          } else if(action == ui::ResourceRowAction::MoveDown) {
            context.push_action([from = i](ApplicationState& app_state) {
              app_state.reorder_list_backtest(from, from + 1);
            });
          } else if(action == ui::ResourceRowAction::Delete) {
            context.push_action([backtest_handle](ApplicationState& app_state) {
              app_state.remove_backtest(backtest_handle);
            });
          }
          ImGui::PopID();
          continue;
        }
      }
    }

    if(backtest_handles.empty()) {
      ImGui::TextDisabled("No backtests yet. Create one to get started.");
    } else if(visible_backtest_count == 0) {
      ImGui::TextDisabled("No backtests match this search.");
    }

    ImGui::EndChild();

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

    const auto valid =
     context.app_state().is_backtest_ready(*self.editing_backtest_ptr_);
    ImGui::BeginDisabled(!valid);
    if(ImGui::Button("Create Backtest")) {
      self.submit_backtest_changes(context, true);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.leave_editor();
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
    const auto same_backtest = selected_backtest == *self.editing_backtest_ptr_;
    const auto valid =
     context.app_state().is_backtest_ready(*self.editing_backtest_ptr_);

    ImGui::BeginDisabled(!valid || same_backtest);
    if(ImGui::Button("Save Backtest")) {
      self.submit_backtest_changes(context, true);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.leave_editor();
    }

    ImGui::SameLine();
    const auto draft_action =
     ui::apply_reset_button(self.selected_draft_action_, !same_backtest, valid);
    if(draft_action == ui::DraftAction::Apply) {
      self.submit_backtest_changes(context, false);
    } else if(draft_action == ui::DraftAction::Reset) {
      self.editing_backtest_ptr_ =
       std::make_shared<backtest::Backtest>(selected_backtest);
    }

    ImGui::EndGroup();
  }

  void edit_backtest_form(this BacktestsWindow& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();

    auto& edit_backtest_ptr = self.editing_backtest_ptr_;
    ui::form_section(
     "Backtest Details",
     "Give this simulation a recognizable name and starting balance. A blank "
     "name is saved as 'Unnamed'.");
    {
      ui::field_label("Name");

      auto backtest_name = edit_backtest_ptr->name();
      ImGui::InputTextWithHint("##NewBacktestName", "Unnamed", &backtest_name);
      edit_backtest_ptr->name(std::move(backtest_name));
    }

    {
      ui::field_label(
       "Initial Capital",
       "Starting cash balance used to size positions and calculate returns.");

      auto initial_capital = edit_backtest_ptr->initial_capital();
      ImGui::InputDouble(
       "##NewInitialCapital", &initial_capital, 100.0, 1000.0, "%.0f");
      edit_backtest_ptr->initial_capital(initial_capital);
    }

    ui::form_section(
     "Backtest Components",
     "Choose the data, trading rules, execution constraints, fees, and "
     "position-sizing profile used by this run.");
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

      ui::field_label("Asset");
      auto asset_preview =
       edit_asset_ptr ? edit_asset_ptr->name() : std::string{"Select an asset"};
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
          const auto strategy_handle = strategy_handles.front();
          backtest::assign_backtest_strategy(
           *edit_backtest_ptr,
           strategy_handle,
           app_state.get_strategy(strategy_handle));
        }
      }

      ui::field_label("Strategy");
      auto strategy_preview = edit_strategy_ptr
                               ? edit_strategy_ptr->name()
                               : std::string{"Select a strategy"};
      if(ImGui::BeginCombo("##StrategyCombo", strategy_preview.c_str())) {
        for(auto i = 0; i < strategy_handles.size(); ++i) {
          const auto& strategy_handle = strategy_handles[i];
          const auto& strategy = app_state.get_strategy(strategy_handle);
          const auto& strategy_name = strategy.name();
          const auto is_selected =
           edit_backtest_ptr->strategy_handle() == strategy_handle;

          ImGui::PushID(i);

          if(ImGui::Selectable(strategy_name.c_str(), is_selected) &&
             !is_selected) {
            backtest::assign_backtest_strategy(
             *edit_backtest_ptr, strategy_handle, strategy);
          }

          if(is_selected) {
            ImGui::SetItemDefaultFocus();
          }

          ImGui::PopID();
        }
        ImGui::EndCombo();
      }

      auto backtest_inputs = edit_backtest_ptr->inputs();
      if(!backtest_inputs.empty()) {
        ImGui::Indent();

        ImGui::SeparatorText("Strategy Inputs");
        ImGui::TextDisabled(
         "Override the named values exposed by the selected strategy.");

        for(auto id_counter = 0; auto& backtest_input : backtest_inputs) {
          ImGui::PushID(id_counter++);

          ui::field_label(backtest_input.label().c_str());

          auto input_value = backtest_input.value();
          switch(backtest_input.representation()) {
          case pludux::NumericInputNode::ValueRepresentation::Decimal: {
            auto editable = input_value;
            if(ImGui::InputScalar(
                "##input_value", ImGuiDataType_Double, &editable)) {
              input_value = editable;
            }
            break;
          }
          case pludux::NumericInputNode::ValueRepresentation::SignedInteger: {
            auto editable = static_cast<std::int64_t>(input_value);
            if(ImGui::InputScalar(
                "##input_value", ImGuiDataType_S64, &editable)) {
              input_value = static_cast<double>(editable);
            }
            break;
          }
          case pludux::NumericInputNode::ValueRepresentation::UnsignedInteger: {
            auto editable = static_cast<std::uint64_t>(input_value);
            if(ImGui::InputScalar(
                "##input_value", ImGuiDataType_U64, &editable)) {
              input_value = static_cast<double>(editable);
            }
            break;
          }
          }

          backtest_input.value(input_value);

          ImGui::Separator();
          ImGui::PopID();
        }

        edit_backtest_ptr->inputs(std::move(backtest_inputs));

        ImGui::Unindent();
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

      ui::field_label("Market");
      auto market_preview = edit_market_ptr ? edit_market_ptr->name()
                                            : std::string{"Select a market"};
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

      ui::field_label("Broker");
      auto broker_preview = edit_broker_ptr ? edit_broker_ptr->name()
                                            : std::string{"Select a broker"};
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

      ui::field_label("Profile");
      auto profile_preview = edit_profile_ptr ? edit_profile_ptr->name()
                                              : std::string{"Select a profile"};
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

    if(!app_state.is_backtest_ready(*edit_backtest_ptr)) {
      ImGui::Spacing();
      ui::validation_message(
       "Select an asset, strategy, market, broker, and profile before saving.");
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

  void leave_editor(this BacktestsWindow& self) noexcept
  {
    self.backtest_panel_mode_ = BacktestPanelMode::List;
  }
};

} // namespace pludux::apps

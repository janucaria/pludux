module;

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.profiles_window;

import pludux.backtest;
import :condition_node_editor;
import :window_context;
import :ui.widgets;

export namespace pludux::apps {

class ProfilesWindow {
public:
  ProfilesWindow()
  : current_page_(ProfilePage::List)
  , selected_profile_handle_opt_{}
  , editing_profile_ptr_{nullptr}
  , last_position_sizing_values_{
     {backtest::PositionSizing::Mode::RiskDistance, 0.01},
     {backtest::PositionSizing::Mode::FixedQuantity, 1.0},
     {backtest::PositionSizing::Mode::FixedNotional, 1000.0},
     {backtest::PositionSizing::Mode::EquityPercent, 1.0}}
  {
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Profiles");
    switch(self.current_page_) {
    case ProfilePage::AddNewProfile:
      self.render_add_new_profile(context);
      break;
    case ProfilePage::EditProfile:
      self.render_edit_profile(context);
      break;
    case ProfilePage::List:
    default:
      self.render_profiles_list(context);
      break;
    }

    ImGui::End();
  }

  void discard_draft(this ProfilesWindow& self) noexcept
  {
    self.reset();
  }

private:
  enum class ProfilePage { List, AddNewProfile, EditProfile } current_page_;

  std::optional<backtest::ProfileStoreHandle> selected_profile_handle_opt_;
  std::shared_ptr<backtest::Profile> editing_profile_ptr_;
  ImGuiTextFilter profile_filter_;
  ui::DraftAction selected_draft_action_{ui::DraftAction::Apply};

  std::unordered_map<backtest::PositionSizing::Mode, double>
   last_position_sizing_values_;

  void render_profiles_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& profile_handles = app_state.get_profile_handles();
    const auto backtest_ptr = app_state.selected_backtest_if_present();

    ImGui::BeginGroup();
    if(ImGui::Button(PLUDUX_ICON_ADD " New Profile")) {
      self.current_page_ = ProfilePage::AddNewProfile;
      self.selected_profile_handle_opt_ = std::nullopt;
      self.editing_profile_ptr_ = std::make_shared<backtest::Profile>();
      self.editing_profile_ptr_->position_sizing(backtest::PositionSizing{
       backtest::PositionSizing::Mode::RiskDistance, 0.01});
    }
    ImGui::Spacing();
    ui::search_filter(self.profile_filter_, "##profiles_search");
    ImGui::BeginChild("item view", ImVec2(0, 0));

    auto visible_profile_count = std::size_t{0};
    for(auto i = 0; i < profile_handles.size(); ++i) {
      const auto profile_handle = profile_handles[i];
      const auto& profile = app_state.get_profile(profile_handle);
      if(!self.profile_filter_.PassFilter(profile.name().c_str())) {
        continue;
      }
      ++visible_profile_count;

      ImGui::PushID(i);

      {
        const auto selected =
         backtest_ptr && backtest_ptr->profile_handle() == profile_handle;
        const auto has_draft =
         self.selected_profile_handle_opt_ == profile_handle &&
         self.editing_profile_ptr_ && *self.editing_profile_ptr_ != profile;
        const auto display_name =
         has_draft ? profile.name() + " (Unsaved)" : profile.name();
        const auto action = ui::resource_row(
         display_name.c_str(), selected, i, profile_handles.size());
        if(action == ui::ResourceRowAction::Edit) {
          self.current_page_ = ProfilePage::EditProfile;
          if(self.selected_profile_handle_opt_ != profile_handle ||
             !self.editing_profile_ptr_) {
            self.selected_profile_handle_opt_ = profile_handle;
            self.editing_profile_ptr_ =
             std::make_shared<backtest::Profile>(profile);
          }
        } else if(action == ui::ResourceRowAction::Duplicate) {
          context.push_action([profile_handle](ApplicationState& app_state) {
            const auto& value = app_state.get_profile(profile_handle);
            auto copy = value;
            copy.name(value.name() + " Copy");
            app_state.add_profile(std::move(copy));
          });
        } else if(action == ui::ResourceRowAction::MoveUp) {
          context.push_action([from = i](ApplicationState& app_state) {
            app_state.reorder_list_profile(from, from - 1);
          });
        } else if(action == ui::ResourceRowAction::MoveDown) {
          context.push_action([from = i](ApplicationState& app_state) {
            app_state.reorder_list_profile(from, from + 1);
          });
        } else if(action == ui::ResourceRowAction::Delete) {
          context.push_action([profile_handle](ApplicationState& app_state) {
            app_state.remove_profile(profile_handle);
          });
        }
        ImGui::PopID();
        continue;
      }
    }

    if(profile_handles.empty()) {
      ImGui::TextDisabled("No profiles yet. Add one to get started.");
    } else if(visible_profile_count == 0) {
      ImGui::TextDisabled("No profiles match this search.");
    }

    ImGui::EndChild();
    ImGui::EndGroup();
  }

  void render_add_new_profile(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Profile");
    ImGui::Separator();

    self.edit_profile_form(context);

    ImGui::EndChild();
    if(ImGui::Button("Create Profile")) {
      self.submit_profile_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::EndGroup();
  }

  void render_edit_profile(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Profile");
    ImGui::Separator();

    self.edit_profile_form(context);

    ImGui::EndChild();

    const auto selected_profile_handle =
     self.selected_profile_handle_opt_.value();
    const auto& selected_profile =
     context.app_state().get_profile(selected_profile_handle);
    const auto same_profile = selected_profile == *(self.editing_profile_ptr_);

    if(ImGui::Button("OK")) {
      self.submit_profile_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.leave_editor();
    }

    ImGui::SameLine();
    const auto draft_action =
     ui::apply_reset_button(self.selected_draft_action_, !same_profile);
    if(draft_action == ui::DraftAction::Apply) {
      self.submit_profile_changes(context);
    } else if(draft_action == ui::DraftAction::Reset) {
      self.editing_profile_ptr_ =
       std::make_shared<backtest::Profile>(selected_profile);
    }

    ImGui::EndGroup();
  }

  void edit_profile_form(this auto& self, WindowContext& context)
  {
    ui::form_section(
     "Profile Details",
     "A profile controls position size and capital protection for a backtest. "
     "A blank name is saved as 'Unnamed'.");
    {
      auto profile_name = self.editing_profile_ptr_->name();
      ui::field_label("Name");
      ImGui::InputTextWithHint("##profile_name", "Unnamed", &profile_name);
      self.editing_profile_ptr_->name(profile_name);
    }

    ui::form_section(
     "Position Sizing",
     "Choose how much capital each new position receives. Only the value for "
     "the selected method is used.");
    {
      auto position_sizing = self.editing_profile_ptr_->position_sizing();
      const auto mode_label = [](backtest::PositionSizing::Mode mode) {
        switch(mode) {
        case backtest::PositionSizing::Mode::RiskDistance:
          return "Risk Distance";
        case backtest::PositionSizing::Mode::FixedQuantity:
          return "Fixed Quantity";
        case backtest::PositionSizing::Mode::FixedNotional:
          return "Fixed Notional";
        case backtest::PositionSizing::Mode::EquityPercent:
          return "Equity Percent";
        }

        return "Risk Distance";
      };

      constexpr auto position_sizing_modes =
       std::array{backtest::PositionSizing::Mode::RiskDistance,
                  backtest::PositionSizing::Mode::FixedQuantity,
                  backtest::PositionSizing::Mode::FixedNotional,
                  backtest::PositionSizing::Mode::EquityPercent};

      auto mode = position_sizing.mode();
      ui::field_label(
       "Position Sizing",
       "Risk Distance limits capital at risk for a 1R move. Fixed Quantity "
       "uses asset units, Fixed Notional uses a currency amount, and Equity "
       "Percent allocates a share of current equity.");
      if(ImGui::BeginCombo("##position_sizing", mode_label(mode))) {
        for(const auto& position_sizing_mode : position_sizing_modes) {
          const auto selected = mode == position_sizing_mode;
          if(ImGui::Selectable(mode_label(position_sizing_mode), selected)) {
            mode = position_sizing_mode;
            const auto value = self.last_position_sizing_values_[mode];
            position_sizing = backtest::PositionSizing{mode, value};
          }
          if(selected) {
            ImGui::SetItemDefaultFocus();
          }
        }

        ImGui::EndCombo();
      }

      auto value = position_sizing.value();
      switch(mode) {
      case backtest::PositionSizing::Mode::RiskDistance: {
        auto percentage = value * 100.0;
        ui::field_label("Capital Risk (%)",
                        "Percent of current capital at risk for a 1R loss.");
        ImGui::InputDouble("##capital_risk", &percentage, 1.0, 10.0, "%.2f");
        value = percentage / 100.0;
        break;
      }
      case backtest::PositionSizing::Mode::FixedQuantity:
        ui::field_label("Quantity", "Number of asset units opened per entry.");
        ImGui::InputDouble("##quantity", &value, 1.0, 10.0, "%.8f");
        break;
      case backtest::PositionSizing::Mode::FixedNotional:
        ui::field_label("Notional", "Currency value allocated to each entry.");
        ImGui::InputDouble("##notional", &value, 100.0, 1000.0, "%.2f");
        break;
      case backtest::PositionSizing::Mode::EquityPercent: {
        auto percentage = value * 100.0;
        ui::field_label("Equity (%)",
                        "Percent of current equity allocated to each entry.");
        ImGui::InputDouble("##equity_percent", &percentage, 1.0, 10.0, "%.2f");
        value = percentage / 100.0;
        break;
      }
      }

      position_sizing.mode(mode);
      position_sizing.value(value);
      self.last_position_sizing_values_[mode] = value;
      self.editing_profile_ptr_->position_sizing(position_sizing);
    }

    ui::form_section(
     "Cash Handling",
     "Choose what happens when the requested position costs more than the "
     "available cash.");
    {
      auto insufficient_cash_policy =
       self.editing_profile_ptr_->insufficient_cash_policy();
      const auto policy_label = [](backtest::InsufficientCashPolicy policy) {
        switch(policy) {
        case backtest::InsufficientCashPolicy::Reject:
          return "Reject Order";
        case backtest::InsufficientCashPolicy::CapToAvailableCash:
          return "Cap To Available Cash";
        }

        return "Reject Order";
      };

      constexpr auto insufficient_cash_policies =
       std::array{backtest::InsufficientCashPolicy::Reject,
                  backtest::InsufficientCashPolicy::CapToAvailableCash};

      ui::field_label(
       "Insufficient Cash",
       "Reject Order skips an unaffordable entry. Cap To Available Cash "
       "reduces its size to the largest affordable order.");
      if(ImGui::BeginCombo("##insufficient_cash",
                           policy_label(insufficient_cash_policy))) {
        for(const auto& policy : insufficient_cash_policies) {
          const auto selected = insufficient_cash_policy == policy;
          if(ImGui::Selectable(policy_label(policy), selected)) {
            insufficient_cash_policy = policy;
          }
          if(selected) {
            ImGui::SetItemDefaultFocus();
          }
        }

        ImGui::EndCombo();
      }

      self.editing_profile_ptr_->insufficient_cash_policy(
       insufficient_cash_policy);
    }

    ui::form_section(
     "Execution Filter",
     "Decide whether each initial strategy entry should become a real "
     "position. Accepted positions automatically mirror later pyramiding and "
     "exit intents.");
    {
      auto filter = self.editing_profile_ptr_->execution_filter();
      ui::field_label("Decision rule");
      if(backtest::gui::execution_filter_node_editor(filter)) {
        self.editing_profile_ptr_->execution_filter(std::move(filter));
      }
    }

    ui::form_section(
     "Drawdown Adjustment",
     "Optionally reduce new position sizes as account drawdown deepens. The "
     "reduction is applied once for each completed drawdown step.");
    {
      auto drawdown_adjustment =
       self.editing_profile_ptr_->drawdown_adjustment();
      auto enabled = drawdown_adjustment.enabled();
      ui::field_label(
       "Drawdown Adjustment",
       "When enabled, new positions become smaller as drawdown passes each "
       "configured step.");
      ImGui::Checkbox("##drawdown_adjustment", &enabled);
      drawdown_adjustment.enabled(enabled);

      ImGui::BeginDisabled(!enabled);
      auto drawdown_step = drawdown_adjustment.drawdown_step() * 100.0;
      ui::field_label(
       "Drawdown step (%)",
       "Drawdown interval that triggers another size reduction.");
      ImGui::InputDouble("##drawdown_step", &drawdown_step, 1.0, 10.0, "%.2f");
      drawdown_adjustment.drawdown_step(drawdown_step / 100.0);

      auto size_reduction = drawdown_adjustment.size_reduction() * 100.0;
      ui::field_label(
       "Size reduction (%)",
       "Amount removed from the base position size per completed drawdown "
       "step.");
      ImGui::InputDouble(
       "##size_reduction", &size_reduction, 1.0, 10.0, "%.2f");
      drawdown_adjustment.size_reduction(size_reduction / 100.0);
      ImGui::EndDisabled();

      self.editing_profile_ptr_->drawdown_adjustment(drawdown_adjustment);
    }
  }

  void submit_profile_changes(this auto& self, WindowContext& context)
  {
    context.push_action([profile_handle_opt = self.selected_profile_handle_opt_,
                         edit_profile_ptr = self.editing_profile_ptr_](
                         ApplicationState& app_state) {
      if(edit_profile_ptr->name().empty()) {
        edit_profile_ptr->name("Unnamed");
      }

      if(!profile_handle_opt.has_value()) {
        app_state.add_profile(*edit_profile_ptr);
        return;
      }

      app_state.update_profile(profile_handle_opt.value(), *edit_profile_ptr);
    });
  }

  void reset(this ProfilesWindow& self) noexcept
  {
    self.current_page_ = ProfilePage::List;
    self.selected_profile_handle_opt_ = std::nullopt;
    self.editing_profile_ptr_ = nullptr;
  }

  void leave_editor(this ProfilesWindow& self) noexcept
  {
    self.current_page_ = ProfilePage::List;
  }
};

} // namespace pludux::apps

module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
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
      self.editing_profile_ptr_->position_sizing(
       backtest::PositionSizingNode{backtest::RiskDistancePositionSizing{}});
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
    const auto draft_action = ui::apply_reset_button(!same_profile);
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
      enum class SizingKind {
        RiskDistance,
        FixedQuantity,
        FixedNotional,
        EquityFraction,
        BayesianKelly
      };
      constexpr auto kinds = std::array{SizingKind::RiskDistance,
                                        SizingKind::FixedQuantity,
                                        SizingKind::FixedNotional,
                                        SizingKind::EquityFraction,
                                        SizingKind::BayesianKelly};
      const auto kind_label = [](SizingKind kind) {
        switch(kind) {
        case SizingKind::RiskDistance:
          return "Risk Distance";
        case SizingKind::FixedQuantity:
          return "Fixed Quantity";
        case SizingKind::FixedNotional:
          return "Fixed Notional";
        case SizingKind::EquityFraction:
          return "Equity Fraction";
        case SizingKind::BayesianKelly:
          return "Bayesian Kelly (Strategy Performance)";
        }
        return "Risk Distance";
      };

      const auto& position_sizing =
       self.editing_profile_ptr_->position_sizing();
      auto kind = SizingKind::RiskDistance;
      if(position_sizing_node_cast<backtest::FixedQuantityPositionSizing>(
          position_sizing)) {
        kind = SizingKind::FixedQuantity;
      } else if(position_sizing_node_cast<
                 backtest::FixedNotionalPositionSizing>(position_sizing)) {
        kind = SizingKind::FixedNotional;
      } else if(position_sizing_node_cast<
                 backtest::EquityFractionPositionSizing>(position_sizing)) {
        kind = SizingKind::EquityFraction;
      } else if(position_sizing_node_cast<
                 backtest::StrategyPerformanceBayesianKellySizing>(
                 position_sizing)) {
        kind = SizingKind::BayesianKelly;
      }

      ui::field_label(
       "Position Sizing",
       "Risk Distance limits capital at risk for a 1R move. Fixed Quantity "
       "uses asset units, Fixed Notional uses a currency amount, Equity "
       "Fraction allocates current equity, and Bayesian Kelly uses the "
       "Bayesian Strategy Performance model.");
      if(ImGui::BeginCombo("##position_sizing", kind_label(kind))) {
        for(const auto candidate : kinds) {
          const auto selected = kind == candidate;
          if(ImGui::Selectable(kind_label(candidate), selected)) {
            kind = candidate;
            switch(kind) {
            case SizingKind::RiskDistance:
              self.editing_profile_ptr_->position_sizing(
               backtest::PositionSizingNode{
                backtest::RiskDistancePositionSizing{}});
              break;
            case SizingKind::FixedQuantity:
              self.editing_profile_ptr_->position_sizing(
               backtest::PositionSizingNode{
                backtest::FixedQuantityPositionSizing{}});
              break;
            case SizingKind::FixedNotional:
              self.editing_profile_ptr_->position_sizing(
               backtest::PositionSizingNode{
                backtest::FixedNotionalPositionSizing{}});
              break;
            case SizingKind::EquityFraction:
              self.editing_profile_ptr_->position_sizing(
               backtest::PositionSizingNode{
                backtest::EquityFractionPositionSizing{}});
              break;
            case SizingKind::BayesianKelly:
              self.editing_profile_ptr_->position_sizing(
               backtest::PositionSizingNode{
                backtest::StrategyPerformanceBayesianKellySizing{}});
              break;
            }
          }
          if(selected) {
            ImGui::SetItemDefaultFocus();
          }
        }

        ImGui::EndCombo();
      }

      const auto assign_if = [&self](bool valid, auto sizing) {
        if(valid) {
          self.editing_profile_ptr_->position_sizing(
           backtest::PositionSizingNode{std::move(sizing)});
        } else {
          ImGui::TextColored(ImVec4{1.0f, 0.4f, 0.4f, 1.0f},
                             "Value is outside the valid range.");
        }
      };
      const auto& current = self.editing_profile_ptr_->position_sizing();
      switch(kind) {
      case SizingKind::RiskDistance: {
        auto value =
         position_sizing_node_cast<backtest::RiskDistancePositionSizing>(
          current)
          ->risk_fraction();
        auto percentage = value * 100.0;
        ui::field_label("Capital Risk (%)",
                        "Percent of current capital at risk for a 1R loss.");
        if(ImGui::InputDouble(
            "##capital_risk", &percentage, 1.0, 10.0, "%.2f")) {
          value = percentage / 100.0;
          assign_if(std::isfinite(value) && value > 0.0,
                    backtest::RiskDistancePositionSizing{
                     std::isfinite(value) && value > 0.0 ? value : 0.01});
        }
        break;
      }
      case SizingKind::FixedQuantity: {
        auto value =
         position_sizing_node_cast<backtest::FixedQuantityPositionSizing>(
          current)
          ->quantity();
        ui::field_label("Quantity", "Number of asset units opened per entry.");
        if(ImGui::InputDouble("##quantity", &value, 1.0, 10.0, "%.8f")) {
          assign_if(std::isfinite(value) && value > 0.0,
                    backtest::FixedQuantityPositionSizing{
                     std::isfinite(value) && value > 0.0 ? value : 1.0});
        }
        break;
      }
      case SizingKind::FixedNotional: {
        auto value =
         position_sizing_node_cast<backtest::FixedNotionalPositionSizing>(
          current)
          ->notional();
        ui::field_label("Notional", "Currency value allocated to each entry.");
        if(ImGui::InputDouble("##notional", &value, 100.0, 1000.0, "%.2f")) {
          assign_if(std::isfinite(value) && value > 0.0,
                    backtest::FixedNotionalPositionSizing{
                     std::isfinite(value) && value > 0.0 ? value : 1000.0});
        }
        break;
      }
      case SizingKind::EquityFraction: {
        auto value =
         position_sizing_node_cast<backtest::EquityFractionPositionSizing>(
          current)
          ->equity_fraction();
        auto percentage = value * 100.0;
        ui::field_label("Equity (%)",
                        "Percent of current equity allocated to each entry.");
        if(ImGui::InputDouble(
            "##equity_fraction", &percentage, 1.0, 10.0, "%.2f")) {
          value = percentage / 100.0;
          assign_if(std::isfinite(value) && value > 0.0,
                    backtest::EquityFractionPositionSizing{
                     std::isfinite(value) && value > 0.0 ? value : 0.01});
        }
        break;
      }
      case SizingKind::BayesianKelly: {
        const auto& value = *position_sizing_node_cast<
         backtest::StrategyPerformanceBayesianKellySizing>(current);
        auto estimate = value.estimate();
        ui::field_label(
         "Posterior Estimate",
         "Adverse Quantiles selects cautious marginal posterior values. "
         "Posterior "
         "Mean uses each model's posterior mean.");
        const auto estimate_label =
         estimate ==
           backtest::StrategyPerformanceBayesianKellyEstimate::PosteriorMean
          ? "Posterior Mean"
          : "Adverse Quantiles";
        if(ImGui::BeginCombo("##kelly_estimate", estimate_label)) {
          if(ImGui::Selectable(
              "Adverse Quantiles",
              estimate == backtest::StrategyPerformanceBayesianKellyEstimate::
                           AdverseQuantiles)) {
            estimate = backtest::StrategyPerformanceBayesianKellyEstimate::
             AdverseQuantiles;
          }
          if(ImGui::Selectable(
              "Posterior Mean",
              estimate == backtest::StrategyPerformanceBayesianKellyEstimate::
                           PosteriorMean)) {
            estimate =
             backtest::StrategyPerformanceBayesianKellyEstimate::PosteriorMean;
          }
          ImGui::EndCombo();
        }
        auto credible_percent = value.central_credible_mass() * 100.0;
        auto multiplier = value.kelly_multiplier();
        auto maximum_percent = value.maximum_equity_fraction() * 100.0;
        ui::field_label(
         "Central Credible Mass (%)",
         "Used by Adverse Quantiles to choose cautious posterior values. For "
         "example, 80% selects the lower 10th percentile for win probability "
         "and winning payoff, and the upper 90th percentile for losing "
         "payoff.");
        ImGui::InputDouble(
         "##kelly_credible", &credible_percent, 1.0, 5.0, "%.2f");
        ui::field_label("Kelly Multiplier", "0 disables execution sizing.");
        ImGui::InputDouble(
         "##kelly_multiplier", &multiplier, 0.05, 0.10, "%.3f");
        ui::field_label("Maximum Equity Per Entry (%)",
                        "May exceed 100%; cash policy still applies.");
        ImGui::InputDouble(
         "##kelly_maximum", &maximum_percent, 5.0, 25.0, "%.2f");
        const auto credible = credible_percent / 100.0;
        const auto maximum = maximum_percent / 100.0;
        const auto valid = std::isfinite(credible) && credible > 0.0 &&
                           credible < 1.0 && std::isfinite(multiplier) &&
                           multiplier >= 0.0 && multiplier <= 1.0 &&
                           std::isfinite(maximum) && maximum > 0.0;
        assign_if(valid,
                  backtest::StrategyPerformanceBayesianKellySizing{
                   estimate,
                   valid ? credible : 0.80,
                   valid ? multiplier : 0.50,
                   valid ? maximum : 1.0});
        ImGui::TextWrapped(
         "Uses theoretical Strategy Performance: Bayesian win probability, "
         "winning payoff magnitude, and losing payoff magnitude.");
        break;
      }
      }
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

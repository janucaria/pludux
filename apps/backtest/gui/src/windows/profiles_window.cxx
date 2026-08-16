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
    const auto system_ptr = app_state.selected_system_if_present();

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
        const auto selected = system_ptr && [&] {
          for(auto strategy_index = std::size_t{};
              strategy_index < system_ptr->strategy_count(); ++strategy_index) {
            const auto* strategy = app_state.get_strategy_if_present(
             system_ptr->strategy_handle(strategy_index));
            if(strategy && strategy->references_profile(profile_handle)) {
              return true;
            }
          }
          return false;
        }();
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
          context.push_edit(
           "Duplicate Profile", [profile_handle](ApplicationState& app_state) {
             const auto& value = app_state.get_profile(profile_handle);
             auto copy = value;
             copy.name(value.name() + " Copy");
             app_state.add_profile(std::move(copy));
           });
        } else if(action == ui::ResourceRowAction::MoveUp) {
          context.push_edit("Move Profile Up",
                            [from = i](ApplicationState& app_state) {
                              app_state.reorder_list_profile(from, from - 1);
                            });
        } else if(action == ui::ResourceRowAction::MoveDown) {
          context.push_edit("Move Profile Down",
                            [from = i](ApplicationState& app_state) {
                              app_state.reorder_list_profile(from, from + 1);
                            });
        } else if(action == ui::ResourceRowAction::Delete) {
          context.push_edit("Delete Profile",
                            [profile_handle](ApplicationState& app_state) {
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
      if(!same_profile) {
        self.submit_profile_changes(context);
      }
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
        FixedBudget,
        EquityFraction,
        BayesianKelly
      };
      constexpr auto kinds = std::array{SizingKind::RiskDistance,
                                        SizingKind::FixedQuantity,
                                        SizingKind::FixedBudget,
                                        SizingKind::EquityFraction,
                                        SizingKind::BayesianKelly};
      const auto kind_label = [](SizingKind kind) {
        switch(kind) {
        case SizingKind::RiskDistance:
          return "Risk Distance";
        case SizingKind::FixedQuantity:
          return "Fixed Quantity";
        case SizingKind::FixedBudget:
          return "Fixed Budget";
        case SizingKind::EquityFraction:
          return "Equity Fraction";
        case SizingKind::BayesianKelly:
          return "Bayesian Kelly (Model Performance)";
        }
        return "Risk Distance";
      };

      const auto& position_sizing =
       self.editing_profile_ptr_->position_sizing();
      auto kind = SizingKind::RiskDistance;
      if(position_sizing_node_cast<backtest::FixedQuantityPositionSizing>(
          position_sizing)) {
        kind = SizingKind::FixedQuantity;
      } else if(position_sizing_node_cast<backtest::FixedBudgetPositionSizing>(
                 position_sizing)) {
        kind = SizingKind::FixedBudget;
      } else if(position_sizing_node_cast<
                 backtest::EquityFractionPositionSizing>(position_sizing)) {
        kind = SizingKind::EquityFraction;
      } else if(position_sizing_node_cast<
                  backtest::ModelPerformanceBayesianKellySizing>(
                 position_sizing)) {
        kind = SizingKind::BayesianKelly;
      }

      ui::field_label(
       "Position Sizing",
       "Risk Distance limits loss including estimated round-trip fees for a "
       "1R move. Fixed Quantity uses asset units, Fixed Budget limits entry "
       "notional plus fees, Equity Fraction allocates current equity, and "
        "Bayesian Kelly uses the Bayesian Model Performance model.");
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
            case SizingKind::FixedBudget:
              self.editing_profile_ptr_->position_sizing(
               backtest::PositionSizingNode{
                backtest::FixedBudgetPositionSizing{}});
              break;
            case SizingKind::EquityFraction:
              self.editing_profile_ptr_->position_sizing(
               backtest::PositionSizingNode{
                backtest::EquityFractionPositionSizing{}});
              break;
            case SizingKind::BayesianKelly:
              self.editing_profile_ptr_->position_sizing(
               backtest::PositionSizingNode{
                 backtest::ModelPerformanceBayesianKellySizing{}});
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
        ui::field_label(
         "Equity Risk (%)",
         "Maximum percent of current equity lost at the risk boundary, "
         "including estimated entry and exit fees.");
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
      case SizingKind::FixedBudget: {
        auto value =
         position_sizing_node_cast<backtest::FixedBudgetPositionSizing>(current)
          ->budget();
        ui::field_label("Budget",
                        "Maximum entry notional plus applicable broker fees.");
        if(ImGui::InputDouble("##budget", &value, 100.0, 1000.0, "%.2f")) {
          assign_if(std::isfinite(value) && value > 0.0,
                    backtest::FixedBudgetPositionSizing{
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
        ui::field_label(
         "Equity (%)",
         "Maximum percent of current equity used by entry notional and fees.");
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
          backtest::ModelPerformanceBayesianKellySizing>(current);
        auto estimate = value.estimate();
        ui::field_label(
         "Posterior Estimate",
         "Adverse Quantiles selects cautious marginal posterior values. "
         "Posterior "
         "Mean uses each model's posterior mean.");
        const auto estimate_label =
         estimate ==
            backtest::ModelPerformanceBayesianKellyEstimate::PosteriorMean
          ? "Posterior Mean"
          : "Adverse Quantiles";
        if(ImGui::BeginCombo("##kelly_estimate", estimate_label)) {
          if(ImGui::Selectable(
              "Adverse Quantiles",
               estimate == backtest::ModelPerformanceBayesianKellyEstimate::
                           AdverseQuantiles)) {
             estimate = backtest::ModelPerformanceBayesianKellyEstimate::
             AdverseQuantiles;
          }
          if(ImGui::Selectable(
              "Posterior Mean",
               estimate == backtest::ModelPerformanceBayesianKellyEstimate::
                           PosteriorMean)) {
            estimate =
              backtest::ModelPerformanceBayesianKellyEstimate::PosteriorMean;
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
        ui::field_label(
         "Maximum Equity Per Entry (%)",
         "Caps entry notional plus fees. May exceed 100%; cash policy still "
         "applies.");
        ImGui::InputDouble(
         "##kelly_maximum", &maximum_percent, 5.0, 25.0, "%.2f");
        const auto credible = credible_percent / 100.0;
        const auto maximum = maximum_percent / 100.0;
        const auto valid = std::isfinite(credible) && credible > 0.0 &&
                           credible < 1.0 && std::isfinite(multiplier) &&
                           multiplier >= 0.0 && multiplier <= 1.0 &&
                           std::isfinite(maximum) && maximum > 0.0;
        assign_if(valid,
                   backtest::ModelPerformanceBayesianKellySizing{
                   estimate,
                   valid ? credible : 0.80,
                   valid ? multiplier : 0.50,
                   valid ? maximum : 1.0});
        ImGui::TextWrapped(
          "Uses theoretical Model Performance: Bayesian win probability, "
         "winning payoff magnitude, and losing payoff magnitude.");
        break;
      }
      }
    }

    ui::form_section(
     "Capital Protection",
     "Control how this Profile responds to portfolio drawdown and orders "
     "that exceed currently available cash.");
    {
      auto adjustment = self.editing_profile_ptr_->drawdown_adjustment();
      auto adjustment_enabled = adjustment.enabled();
      ui::field_label("Drawdown adjustment");
      ImGui::Checkbox("##profile_drawdown_enabled", &adjustment_enabled);
      adjustment.enabled(adjustment_enabled);
      ImGui::BeginDisabled(!adjustment_enabled);
      auto step_percent = adjustment.drawdown_step() * 100.0;
      auto reduction_percent = adjustment.size_reduction() * 100.0;
      auto notional_equity_reduction_percent =
       adjustment.notional_equity_reduction() * 100.0;
      ui::field_label("Drawdown step (%)");
      ImGui::InputDouble("##profile_drawdown_step", &step_percent);
      ui::field_label("Size reduction (%)");
      ImGui::InputDouble("##profile_size_reduction", &reduction_percent);
      ui::field_label("Notional equity reduction (%)");
      ImGui::InputDouble("##profile_notional_equity_reduction",
                         &notional_equity_reduction_percent);
      ImGui::TextWrapped(
       "Reduces peak equity by this percentage per completed drawdown step "
       "before equity-dependent position sizing is evaluated.");
      ImGui::EndDisabled();
      if(std::isfinite(step_percent) && step_percent > 0.0) {
        adjustment.drawdown_step(step_percent / 100.0);
      }
      if(std::isfinite(reduction_percent) && reduction_percent >= 0.0) {
        adjustment.size_reduction(reduction_percent / 100.0);
      }
      if(std::isfinite(notional_equity_reduction_percent) &&
         notional_equity_reduction_percent >= 0.0) {
        adjustment.notional_equity_reduction(notional_equity_reduction_percent /
                                             100.0);
      }
      self.editing_profile_ptr_->drawdown_adjustment(adjustment);

      auto cash_policy = self.editing_profile_ptr_->insufficient_cash_policy();
      ui::field_label(
       "Insufficient cash",
       "Reject preserves the requested quantity. Cap submits the largest "
       "Market-valid quantity affordable with available portfolio cash.");
      const auto* cash_label =
       cash_policy == backtest::InsufficientCashPolicy::Reject
        ? "Reject Order"
        : "Cap To Available Cash";
      if(ImGui::BeginCombo("##profile_cash", cash_label)) {
        if(ImGui::Selectable("Reject Order",
                             cash_policy ==
                              backtest::InsufficientCashPolicy::Reject)) {
          cash_policy = backtest::InsufficientCashPolicy::Reject;
        }
        if(ImGui::Selectable(
            "Cap To Available Cash",
            cash_policy ==
             backtest::InsufficientCashPolicy::CapToAvailableCash)) {
          cash_policy = backtest::InsufficientCashPolicy::CapToAvailableCash;
        }
        ImGui::EndCombo();
      }
      self.editing_profile_ptr_->insufficient_cash_policy(cash_policy);
    }
  }

  void submit_profile_changes(this auto& self, WindowContext& context)
  {
    context.push_edit(
     self.selected_profile_handle_opt_ ? "Edit Profile" : "Add Profile",
     [profile_handle_opt = self.selected_profile_handle_opt_,
      edit_profile_ptr =
       self.editing_profile_ptr_](ApplicationState& app_state) {
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

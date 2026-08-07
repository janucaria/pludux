module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
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
  std::string configuration_error_;

  void render_backtests_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& backtest_handles = app_state.get_backtest_handles();

    ImGui::BeginGroup();
    if(ImGui::Button(PLUDUX_ICON_ADD " New Backtest")) {
      self.backtest_panel_mode_ = BacktestPanelMode::AddNew;
      self.selected_backtest_handle_opt_ = std::nullopt;
      self.editing_backtest_ptr_ = std::make_shared<backtest::Backtest>();
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
            context.push_view_action(
             [backtest_handle](ApplicationState& app_state) {
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
            context.push_edit("Duplicate Backtest",
                              [backtest_handle](ApplicationState& app_state) {
                                const auto& value =
                                 app_state.get_backtest(backtest_handle);
                                auto copy = value;
                                copy.name(value.name() + " Copy");
                                app_state.add_backtest(std::move(copy));
                              });
          } else if(action == ui::ResourceRowAction::MoveUp) {
            context.push_edit("Move Backtest Up",
                              [from = i](ApplicationState& app_state) {
                                app_state.reorder_list_backtest(from, from - 1);
                              });
          } else if(action == ui::ResourceRowAction::MoveDown) {
            context.push_edit("Move Backtest Down",
                              [from = i](ApplicationState& app_state) {
                                app_state.reorder_list_backtest(from, from + 1);
                              });
          } else if(action == ui::ResourceRowAction::Delete) {
            context.push_edit("Delete Backtest",
                              [backtest_handle](ApplicationState& app_state) {
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
    const auto same_backtest = selected_backtest == *self.editing_backtest_ptr_;
    const auto valid =
     context.app_state().is_backtest_ready(*self.editing_backtest_ptr_);

    ImGui::BeginDisabled(!valid);
    if(ImGui::Button("OK")) {
      self.submit_backtest_changes(context, true);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.leave_editor();
    }

    ImGui::SameLine();
    const auto draft_action = ui::apply_reset_button(!same_backtest, valid);
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
     "Give this reusable Watchlist + Strategy + Profile setup a recognizable "
     "name. A blank name is saved as 'Unnamed'.");
    {
      ui::field_label("Name");

      auto backtest_name = edit_backtest_ptr->name();
      ImGui::InputTextWithHint("##NewBacktestName", "Unnamed", &backtest_name);
      edit_backtest_ptr->name(std::move(backtest_name));
    }

    ui::form_section(
     "Backtest Setup",
     "Choose the data, trading rules, and position-sizing profile used by "
     "this reusable backtest.");
    {
      const auto& watchlist_handles = app_state.get_watchlist_handles();
      const auto edit_watchlist_handle = edit_backtest_ptr->watchlist_handle();
      const auto* edit_watchlist =
       app_state.get_watchlist_if_present(edit_watchlist_handle);

      if(!edit_watchlist) {
        if(!self.selected_backtest_handle_opt_ && !watchlist_handles.empty()) {
          edit_backtest_ptr->watchlist_handle(watchlist_handles.front());
        }
      }

      ui::field_label("Watchlist");
      auto watchlist_preview = edit_watchlist
                                ? edit_watchlist->name()
                                : std::string{"Select a watchlist"};
      if(ImGui::BeginCombo("##WatchlistCombo", watchlist_preview.c_str())) {
        for(auto i = 0; i < watchlist_handles.size(); ++i) {
          const auto watchlist_handle = watchlist_handles[i];
          const auto& watchlist = app_state.get_watchlist(watchlist_handle);
          const auto is_selected =
           edit_backtest_ptr->watchlist_handle() == watchlist_handle;

          ImGui::PushID(i);

          if(ImGui::Selectable(watchlist.name().c_str(), is_selected)) {
            edit_backtest_ptr->watchlist_handle(watchlist_handle);
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

    ui::form_section(
     "Strategy Performance Calculation",
     "Configure how Strategy Performance is calculated from completed "
     "theoretical positions. These Bayesian models estimate win probability "
     "and payoff magnitudes; they do not control Strategy signals, execution, "
     "or position sizing.");
    {
      const auto& config = edit_backtest_ptr->strategy_performance();
      const auto* win_model =
       bayesian_model_node_cast<backtest::BetaBernoulliModelNode>(
        config.bayesian().win_probability_model());
      const auto* winning_payoff_model =
       bayesian_model_node_cast<backtest::GammaPayoffModelNode>(
        config.bayesian().winning_payoff_model());
      const auto* losing_payoff_model =
       bayesian_model_node_cast<backtest::GammaPayoffModelNode>(
        config.bayesian().losing_payoff_model());
      if(win_model == nullptr || winning_payoff_model == nullptr ||
         losing_payoff_model == nullptr) {
        ui::validation_message(
         "The selected Bayesian model is not supported by this editor.");
        return;
      }

      auto history_mode = static_cast<int>(config.history().mode());
      auto break_even_treatment =
       static_cast<int>(config.break_even_treatment());
      auto rolling_window = static_cast<int>(config.history().rolling_window());
      auto exponential_decay = config.history().exponential_decay();
      auto prior_win_probability = win_model->prior_probability();
      auto prior_win_strength = win_model->prior_strength();
      auto winning_prior_mean = winning_payoff_model->prior_mean_magnitude();
      auto winning_prior_strength = winning_payoff_model->prior_strength();
      auto winning_cv = winning_payoff_model->coefficient_of_variation();
      auto losing_prior_mean = losing_payoff_model->prior_mean_magnitude();
      auto losing_prior_strength = losing_payoff_model->prior_strength();
      auto losing_cv = losing_payoff_model->coefficient_of_variation();

      constexpr const char* history_modes[] = {
       "All history", "Rolling window", "Exponential decay"};
      auto changed = false;
      ui::field_label("History mode");
      changed |= ImGui::Combo("##strategy_history_mode",
                              &history_mode,
                              history_modes,
                              IM_ARRAYSIZE(history_modes));
      if(history_mode ==
         static_cast<int>(
          backtest::StrategyPerformanceHistoryMode::RollingWindow)) {
        ui::field_label("Rolling window");
        changed |=
         ImGui::InputInt("##strategy_history_window", &rolling_window);
      } else if(history_mode ==
                static_cast<int>(
                 backtest::StrategyPerformanceHistoryMode::ExponentialDecay)) {
        ui::field_label("Evidence decay");
        changed |= ImGui::InputDouble(
         "##strategy_history_decay", &exponential_decay, 0.001, 0.01, "%.4f");
      }

      ImGui::SeparatorText("Strategy Performance: Bayesian win probability");
      constexpr const char* win_models[] = {"Beta-Bernoulli"};
      auto win_model_index = 0;
      ui::field_label("Model");
      changed |= ImGui::Combo("##bayesian_win_model",
                              &win_model_index,
                              win_models,
                              IM_ARRAYSIZE(win_models));
      constexpr const char* break_even_treatments[] = {
       "Skip", "Count as win", "Count as loss"};
      ui::field_label(
       "Break-even treatment",
       "Controls how an exactly zero theoretical return contributes to the "
       "Bayesian win-probability model. Frequentist outcome rates preserve "
       "break-even as its own category, and payoff magnitudes are unchanged.");
      changed |= ImGui::Combo("##strategy_break_even_treatment",
                              &break_even_treatment,
                              break_even_treatments,
                              IM_ARRAYSIZE(break_even_treatments));
      ui::field_label("Prior win probability");
      changed |= ImGui::InputDouble(
       "##prior_win_probability", &prior_win_probability, 0.01, 0.1, "%.4f");
      ui::field_label("Prior strength");
      changed |= ImGui::InputDouble(
       "##prior_win_strength", &prior_win_strength, 0.1, 1.0, "%.4f");

      const auto edit_payoff_model =
       [&changed](const char* heading,
                  const char* id_suffix,
                  double& prior_mean,
                  double& prior_strength,
                  double& coefficient_of_variation) {
         ImGui::SeparatorText(heading);
         ui::field_label("Model");
         ImGui::TextUnformatted("Gamma / Inverse-Gamma");
         ui::field_label("Prior mean magnitude");
         changed |=
          ImGui::InputDouble(std::format("##prior_mean_{}", id_suffix).c_str(),
                             &prior_mean,
                             0.001,
                             0.01,
                             "%.4f");
         ui::field_label("Prior strength");
         changed |= ImGui::InputDouble(
          std::format("##prior_strength_{}", id_suffix).c_str(),
          &prior_strength,
          0.001,
          0.01,
          "%.4f");
         ui::field_label("Coefficient of variation");
         changed |= ImGui::InputDouble(
          std::format("##coefficient_of_variation_{}", id_suffix).c_str(),
          &coefficient_of_variation,
          0.01,
          0.1,
          "%.4f");
       };
      edit_payoff_model("Strategy Performance: Bayesian winning payoff",
                        "winning_payoff",
                        winning_prior_mean,
                        winning_prior_strength,
                        winning_cv);
      edit_payoff_model("Strategy Performance: Bayesian losing payoff",
                        "losing_payoff",
                        losing_prior_mean,
                        losing_prior_strength,
                        losing_cv);

      if(changed) {
        try {
          edit_backtest_ptr->strategy_performance(
           backtest::StrategyPerformanceConfig{
            backtest::StrategyPerformanceHistoryPolicy{
             static_cast<backtest::StrategyPerformanceHistoryMode>(
              history_mode),
             static_cast<std::size_t>(std::max(rolling_window, 0)),
             exponential_decay},
            backtest::StrategyPerformanceBayesianConfig{
             backtest::BayesianWinModelNode{backtest::BetaBernoulliModelNode{
              prior_win_probability, prior_win_strength}},
             backtest::BayesianPayoffModelNode{backtest::GammaPayoffModelNode{
              winning_prior_mean, winning_prior_strength, winning_cv}},
             backtest::BayesianPayoffModelNode{backtest::GammaPayoffModelNode{
              losing_prior_mean, losing_prior_strength, losing_cv}}},
            static_cast<backtest::StrategyPerformanceBreakEvenTreatment>(
             break_even_treatment)});
          self.configuration_error_.clear();
        } catch(const std::exception& error) {
          self.configuration_error_ = error.what();
        }
      }
    }

    if(!self.configuration_error_.empty()) {
      ui::validation_message(self.configuration_error_.c_str());
    }

    if(!app_state.is_backtest_ready(*edit_backtest_ptr)) {
      ImGui::Spacing();
      ui::validation_message(
       "Select a non-empty watchlist, strategy, and profile before saving.");
    }
  }

  void submit_backtest_changes(this auto& self,
                               WindowContext& context,
                               bool reset_on_success)
  {
    auto& edit_backtest_ptr = self.editing_backtest_ptr_;
    if(context.app_state().is_backtest_ready(*edit_backtest_ptr)) {
      context.push_edit(
       self.selected_backtest_handle_opt_ ? "Edit Backtest" : "Add Backtest",
       [backtest_handle_opt = self.selected_backtest_handle_opt_,
        edit_backtest_ptr](ApplicationState& app_state) {
         if(edit_backtest_ptr->name().empty()) {
           edit_backtest_ptr->name("Unnamed");
         }

         if(!backtest_handle_opt) {
           const auto edit_handle_opt =
            app_state.add_backtest(*edit_backtest_ptr);
           if(edit_handle_opt) {
             app_state.select_backtest(*edit_handle_opt);
           }
         } else {
           const auto backtest_handle = backtest_handle_opt.value();
           auto& backtest = app_state.get_backtest(backtest_handle);
           const auto rule_changed =
            !backtest.equivalent_rules(*edit_backtest_ptr);
           if(rule_changed) {
             app_state.update_backtest(*backtest_handle_opt,
                                       *edit_backtest_ptr);
           } else {
             backtest.name(edit_backtest_ptr->name());
           }
         }
       });

      if(reset_on_success) {
        self.reset();
      }
    } else {
      context.alert(
       "Please select a non-empty watchlist, a strategy, and a profile.");
    }
  }

  void reset(this BacktestsWindow& self) noexcept
  {
    self.backtest_panel_mode_ = BacktestPanelMode::List;
    self.selected_backtest_handle_opt_ = std::nullopt;
    self.editing_backtest_ptr_ = nullptr;
    self.configuration_error_.clear();
  }

  void leave_editor(this BacktestsWindow& self) noexcept
  {
    self.backtest_panel_mode_ = BacktestPanelMode::List;
  }
};

} // namespace pludux::apps

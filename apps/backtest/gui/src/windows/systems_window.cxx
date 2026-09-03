module;

#include <algorithm>
#include <cstddef>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.systems_window;

import pludux.backtest;
import :window_context;
import :ui.widgets;

export namespace pludux::apps {

class SystemsWindow {
public:
  void render(this SystemsWindow& self, WindowContext& context)
  {
    ImGui::Begin("Systems");
    if(self.editing_) {
      self.render_editor(context);
    } else {
      self.render_list(context);
    }
    ImGui::End();
  }

  void discard_draft(this SystemsWindow& self) noexcept { self.reset(); }

private:
  std::optional<backtest::SystemStoreHandle> editing_handle_;
  std::shared_ptr<backtest::System> editing_;
  ImGuiTextFilter filter_;
  std::string configuration_error_;

  void render_list(this SystemsWindow& self, WindowContext& context)
  {
    const auto& state = context.app_state();
    const auto& handles = state.get_system_handles();
    if(ImGui::Button(PLUDUX_ICON_ADD " New System")) {
      self.editing_handle_.reset();
      self.editing_ = std::make_shared<backtest::System>();
      self.assign_defaults(state, *self.editing_);
    }
    ImGui::Spacing();
    ui::search_filter(self.filter_, "##systems_search");
    ImGui::BeginChild("systems");
    auto visible = std::size_t{};
    for(auto index = std::size_t{}; index < handles.size(); ++index) {
      const auto handle = handles[index];
      const auto& system = state.get_system(handle);
      if(!self.filter_.PassFilter(system.name().c_str())) continue;
      ++visible;
      ImGui::PushID(static_cast<int>(index));
      const auto action = ui::resource_row(system.name().c_str(),
                                           state.selected_system_handle() == handle,
                                           index,
                                           handles.size());
      if(action == ui::ResourceRowAction::Select) {
        context.push_view_action([handle](ApplicationState& candidate) {
          candidate.select_system(handle);
        });
      } else if(action == ui::ResourceRowAction::Edit) {
        self.editing_handle_ = handle;
        self.editing_ = std::make_shared<backtest::System>(system);
        self.configuration_error_.clear();
      } else if(action == ui::ResourceRowAction::Duplicate) {
        context.push_edit("Duplicate System", [handle](ApplicationState& candidate) {
          auto copy = candidate.get_system(handle);
          copy.name(copy.name() + " Copy");
          candidate.add_system(std::move(copy));
        });
      } else if(action == ui::ResourceRowAction::MoveUp) {
        context.push_edit("Move System Up", [index](ApplicationState& candidate) {
          candidate.reorder_list_system(index, index - 1);
        });
      } else if(action == ui::ResourceRowAction::MoveDown) {
        context.push_edit("Move System Down", [index](ApplicationState& candidate) {
          candidate.reorder_list_system(index, index + 1);
        });
      } else if(action == ui::ResourceRowAction::Delete) {
        context.push_edit("Delete System", [handle](ApplicationState& candidate) {
          candidate.remove_system(handle);
        });
      }
      ImGui::PopID();
    }
    if(handles.empty()) {
      ImGui::TextDisabled("No systems yet. Create one to get started.");
    } else if(visible == 0) {
      ImGui::TextDisabled("No systems match this search.");
    }
    ImGui::EndChild();
  }

  void assign_defaults(this SystemsWindow&, const ApplicationState& state,
                       backtest::System& system)
  {
    if(!state.get_watchlist_handles().empty())
      system.watchlist_handle(state.get_watchlist_handles().front());
    if(!state.get_strategy_handles().empty())
      system.main_strategy_handle(state.get_strategy_handles().front());
  }

  void render_editor(this SystemsWindow& self, WindowContext& context)
  {
    auto& state = context.app_state();
    auto& system = *self.editing_;
    ImGui::TextUnformatted(self.editing_handle_ ? "Edit System" : "Add New System");
    ImGui::Separator();
    auto name = system.name();
    ImGui::InputTextWithHint("Name", "Unnamed", &name);
    system.name(std::move(name));
    self.render_watchlist(state, system);
    self.render_model_performance(system);

    auto main_strategy_handle = system.main_strategy_handle();
    self.render_strategy_picker(state, main_strategy_handle, "Main Strategy");
    system.main_strategy_handle(main_strategy_handle);
    std::optional<std::size_t> remove_failsafe;
    std::optional<std::pair<std::size_t, std::size_t>> reorder_failsafe;
    for(auto index = std::size_t{}; index < system.failsafe_strategies().size(); ++index) {
      ImGui::PushID(static_cast<int>(index));
      auto& failsafe = system.failsafe_strategies()[index];
      auto activation = static_cast<int>(failsafe.activation());
      constexpr const char* activation_modes[] = {
       "Always", "Previous Strategy has entry-filtered theoretical position"};
      ImGui::SeparatorText(std::format("Failsafe Strategy {}", index + 1).c_str());
      ui::field_label("Activation",
                      "An entry-filtered theoretical position remains active until the "
                      "previous Strategy fully closes it. The failsafe still requires its "
                      "own fresh initial-entry signal.");
      if(ImGui::Combo("##failsafe_activation", &activation, activation_modes,
                      IM_ARRAYSIZE(activation_modes))) {
        failsafe.activation(static_cast<backtest::FailsafeStrategyActivation>(activation));
      }
      auto strategy_handle = failsafe.strategy_handle();
      self.render_strategy_picker(state, strategy_handle, "Strategy");
      failsafe.strategy_handle(strategy_handle);
      if(index > 0 && ImGui::Button("Move up"))
        reorder_failsafe = std::pair{index, index - 1};
      if(index > 0) ImGui::SameLine();
      if(index + 1 < system.failsafe_strategies().size() && ImGui::Button("Move down"))
        reorder_failsafe = std::pair{index, index + 1};
      if(index + 1 < system.failsafe_strategies().size()) ImGui::SameLine();
      if(ImGui::Button("Remove failsafe")) remove_failsafe = index;
      ImGui::PopID();
    }
    if(reorder_failsafe) {
      auto& failsafes = system.failsafe_strategies();
      std::swap(failsafes[reorder_failsafe->first], failsafes[reorder_failsafe->second]);
    } else if(remove_failsafe) {
      auto& failsafes = system.failsafe_strategies();
      failsafes.erase(failsafes.begin() + static_cast<std::ptrdiff_t>(*remove_failsafe));
    }
    if(ImGui::Button("Add Failsafe Strategy")) {
      const auto handle = state.get_strategy_handles().empty()
                       ? backtest::StrategyStoreHandle{}
                       : state.get_strategy_handles().front();
      system.failsafe_strategies().emplace_back(handle);
    }
    if(!self.configuration_error_.empty())
      ui::validation_message(self.configuration_error_.c_str());

    const auto valid = state.is_system_ready(system);
    ImGui::BeginDisabled(!valid);
    if(ImGui::Button(self.editing_handle_ ? "OK" : "Create System"))
      self.submit(context, true);
    ImGui::EndDisabled();
    ImGui::SameLine();
    if(ImGui::Button("Cancel")) self.reset();
    if(self.editing_handle_) {
      const auto& original = state.get_system(*self.editing_handle_);
      ImGui::SameLine();
      const auto has_draft = system.name() != original.name() ||
                             !system.equivalent_rules(original);
      const auto draft_action = ui::apply_reset_button(has_draft, valid);
      if(draft_action == ui::DraftAction::Apply) self.submit(context, false);
      else if(draft_action == ui::DraftAction::Reset)
        self.editing_ = std::make_shared<backtest::System>(original);
    }
    if(!valid)
      ui::validation_message("Select a non-empty watchlist and complete stored strategies before saving.");
  }

  void render_watchlist(this SystemsWindow&, const ApplicationState& state,
                        backtest::System& system)
  {
    ImGui::SeparatorText("Watchlist");
    const auto* selected = state.get_watchlist_if_present(system.watchlist_handle());
    if(ImGui::BeginCombo("##watchlist", selected ? selected->name().c_str() : "Select a watchlist")) {
      for(const auto handle : state.get_watchlist_handles()) {
        const auto& watchlist = state.get_watchlist(handle);
        if(ImGui::Selectable(watchlist.name().c_str(), handle == system.watchlist_handle()))
          system.watchlist_handle(handle);
      }
      ImGui::EndCombo();
    }
  }

  void render_strategy_picker(this SystemsWindow&,
                              const ApplicationState& state,
                              backtest::StrategyStoreHandle& handle,
                              const char* title)
  {
    ImGui::SeparatorText(title);
    const auto* strategy = state.get_strategy_if_present(handle);
    ui::field_label("Strategy");
    if(ImGui::BeginCombo("##strategy",
                         strategy ? strategy->name().c_str() : "Select a strategy")) {
      for(const auto candidate : state.get_strategy_handles()) {
        const auto& item = state.get_strategy(candidate);
        if(ImGui::Selectable(item.name().c_str(), candidate == handle)) handle = candidate;
      }
      ImGui::EndCombo();
    }
  }

  void render_model_performance(this SystemsWindow& self, backtest::System& system)
  {
    ui::form_section("Model Performance Calculation",
                     "Configure the shared Model Performance calculation for every "
                     "Strategy in this System.");
    const auto& config = system.model_performance();
    const auto* win = bayesian_model_node_cast<backtest::BetaBernoulliModelNode>(
     config.bayesian().win_probability_model());
    const auto* winning = bayesian_model_node_cast<backtest::GammaPayoffModelNode>(
     config.bayesian().winning_payoff_model());
    const auto* losing = bayesian_model_node_cast<backtest::GammaPayoffModelNode>(
     config.bayesian().losing_payoff_model());
    if(!win || !winning || !losing) {
      ui::validation_message("The selected Bayesian model is not supported by this editor.");
      return;
    }
    auto history_mode = static_cast<int>(config.history().mode());
    auto break_even = static_cast<int>(config.break_even_treatment());
    auto window = static_cast<int>(config.history().rolling_window());
    auto decay = config.history().exponential_decay();
    auto win_probability = win->prior_probability();
    auto win_strength = win->prior_strength();
    auto winning_mean = winning->prior_mean_magnitude();
    auto winning_strength = winning->prior_strength();
    auto winning_cv = winning->coefficient_of_variation();
    auto losing_mean = losing->prior_mean_magnitude();
    auto losing_strength = losing->prior_strength();
    auto losing_cv = losing->coefficient_of_variation();
    constexpr const char* history_modes[] = {"All history", "Rolling window", "Exponential decay"};
    constexpr const char* break_even_treatments[] = {"Skip", "Count as win", "Count as loss"};
    auto changed = false;
    ui::field_label("History mode");
    changed |= ImGui::Combo("##history_mode", &history_mode, history_modes, IM_ARRAYSIZE(history_modes));
    if(history_mode == static_cast<int>(backtest::ModelPerformanceHistoryMode::RollingWindow)) {
      ui::field_label("Rolling window");
      changed |= ImGui::InputInt("##history_window", &window);
    } else if(history_mode == static_cast<int>(backtest::ModelPerformanceHistoryMode::ExponentialDecay)) {
      ui::field_label("Evidence decay");
      changed |= ImGui::InputDouble("##history_decay", &decay, 0.001, 0.01, "%.4f");
    }
    ui::field_label("Break-even treatment");
    changed |= ImGui::Combo("##break_even", &break_even, break_even_treatments,
                            IM_ARRAYSIZE(break_even_treatments));
    ui::field_label("Prior win probability");
    changed |= ImGui::InputDouble("##win_probability", &win_probability, 0.01, 0.1, "%.4f");
    ui::field_label("Prior strength");
    changed |= ImGui::InputDouble("##win_strength", &win_strength, 0.1, 1.0, "%.4f");
    const auto edit_payoff = [&changed](const char* label, const char* id, double& mean,
                                        double& strength, double& cv) {
      ImGui::SeparatorText(label);
      ui::field_label("Prior mean magnitude");
      changed |= ImGui::InputDouble(std::format("##{}_mean", id).c_str(), &mean, 0.001, 0.01, "%.4f");
      ui::field_label("Prior strength");
      changed |= ImGui::InputDouble(std::format("##{}_strength", id).c_str(), &strength, 0.001, 0.01, "%.4f");
      ui::field_label("Coefficient of variation");
      changed |= ImGui::InputDouble(std::format("##{}_cv", id).c_str(), &cv, 0.01, 0.1, "%.4f");
    };
    edit_payoff("Bayesian winning payoff", "winning", winning_mean, winning_strength, winning_cv);
    edit_payoff("Bayesian losing payoff", "losing", losing_mean, losing_strength, losing_cv);
    if(changed) {
      try {
        system.model_performance(backtest::ModelPerformanceConfig{
         backtest::ModelPerformanceHistoryPolicy{
          static_cast<backtest::ModelPerformanceHistoryMode>(history_mode),
          static_cast<std::size_t>(std::max(window, 0)), decay},
         backtest::ModelPerformanceBayesianConfig{
          backtest::BayesianWinModelNode{backtest::BetaBernoulliModelNode{win_probability, win_strength}},
          backtest::BayesianPayoffModelNode{backtest::GammaPayoffModelNode{winning_mean, winning_strength, winning_cv}},
          backtest::BayesianPayoffModelNode{backtest::GammaPayoffModelNode{losing_mean, losing_strength, losing_cv}}},
         static_cast<backtest::ModelPerformanceBreakEvenTreatment>(break_even)});
        self.configuration_error_.clear();
      } catch(const std::exception& error) {
        self.configuration_error_ = error.what();
      }
    }
  }

  void submit(this SystemsWindow& self, WindowContext& context, bool reset_on_success)
  {
    auto value = *self.editing_;
    if(!context.app_state().is_system_ready(value)) return;
    if(value.name().empty()) value.name("Unnamed");
    context.push_edit(self.editing_handle_ ? "Edit System" : "Add System",
                      [handle = self.editing_handle_, value = std::move(value)]
                      (ApplicationState& candidate) mutable {
      if(handle) candidate.update_system(*handle, std::move(value));
      else if(const auto added = candidate.add_system(std::move(value))) candidate.select_system(*added);
    });
    if(reset_on_success) self.reset();
  }

  void reset(this SystemsWindow& self) noexcept
  {
    self.editing_handle_.reset();
    self.editing_.reset();
    self.configuration_error_.clear();
  }
};

} // namespace pludux::apps

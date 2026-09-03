module;

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.strategies_window;

import pludux.backtest;
import :condition_node_editor;
import :window_context;
import :ui.widgets;

export namespace pludux::apps {

class StrategiesWindow {
public:
  void render(this StrategiesWindow& self, WindowContext& context)
  {
    ImGui::Begin("Strategies");
    if(self.editing_) self.render_editor(context);
    else self.render_list(context);
    ImGui::End();
  }

  void discard_draft(this StrategiesWindow& self) noexcept { self.reset(); }

private:
  std::optional<backtest::StrategyStoreHandle> editing_handle_;
  std::shared_ptr<backtest::Strategy> editing_;
  ImGuiTextFilter filter_;

  void render_list(this StrategiesWindow& self, WindowContext& context)
  {
    const auto& state = context.app_state();
    const auto& handles = state.get_strategy_handles();
    if(ImGui::Button(PLUDUX_ICON_ADD " New Strategy")) {
      self.editing_handle_.reset();
      self.editing_ = std::make_shared<backtest::Strategy>();
      self.assign_defaults(state, *self.editing_);
    }
    ImGui::Spacing();
    ui::search_filter(self.filter_, "##strategies_search");
    ImGui::BeginChild("strategies");
    auto visible = std::size_t{};
    for(auto index = std::size_t{}; index < handles.size(); ++index) {
      const auto handle = handles[index];
      const auto& strategy = state.get_strategy(handle);
      if(!self.filter_.PassFilter(strategy.name().c_str())) continue;
      ++visible;
      ImGui::PushID(static_cast<int>(index));
      const auto action = ui::resource_row(strategy.name().c_str(),
                                           state.selected_strategy_handle() == handle,
                                           index,
                                           handles.size());
      if(action == ui::ResourceRowAction::Select) {
        context.push_view_action([handle](ApplicationState& candidate) {
          candidate.select_strategy(handle);
        });
      } else if(action == ui::ResourceRowAction::Edit) {
        self.editing_handle_ = handle;
        self.editing_ = std::make_shared<backtest::Strategy>(strategy);
      } else if(action == ui::ResourceRowAction::Duplicate) {
        context.push_edit("Duplicate Strategy", [handle](ApplicationState& candidate) {
          auto copy = candidate.get_strategy(handle);
          copy.name(copy.name() + " Copy");
          candidate.add_strategy(std::move(copy));
        });
      } else if(action == ui::ResourceRowAction::MoveUp) {
        context.push_edit("Move Strategy Up", [index](ApplicationState& candidate) {
          candidate.reorder_list_strategy(index, index - 1);
        });
      } else if(action == ui::ResourceRowAction::MoveDown) {
        context.push_edit("Move Strategy Down", [index](ApplicationState& candidate) {
          candidate.reorder_list_strategy(index, index + 1);
        });
      } else if(action == ui::ResourceRowAction::Delete) {
        context.push_edit("Delete Strategy", [handle](ApplicationState& candidate) {
          candidate.remove_strategy(handle);
        });
      }
      const auto dependent_systems = self.dependent_system_count(state, handle);
      if(dependent_systems != 0) {
        ImGui::TextDisabled("Referenced by %zu system%s; deletion leaves those references broken.",
                            dependent_systems,
                            dependent_systems == 1 ? "" : "s");
      }
      ImGui::PopID();
    }
    if(handles.empty()) ImGui::TextDisabled("No strategies yet. Create one to get started.");
    else if(visible == 0) ImGui::TextDisabled("No strategies match this search.");
    ImGui::EndChild();
  }

  void assign_defaults(this StrategiesWindow&, const ApplicationState& state,
                       backtest::Strategy& strategy)
  {
    if(!state.get_profile_handles().empty())
      strategy.profile_handle(state.get_profile_handles().front());
    if(!state.get_model_handles().empty()) {
      const auto handle = state.get_model_handles().front();
      backtest::assign_strategy_model(strategy, handle, state.get_model(handle));
    }
  }

  void render_editor(this StrategiesWindow& self, WindowContext& context)
  {
    auto& state = context.app_state();
    auto& strategy = *self.editing_;
    ImGui::TextUnformatted(self.editing_handle_ ? "Edit Strategy" : "Add New Strategy");
    ImGui::Separator();
    self.render_name(strategy);
    self.render_model(state, strategy);
    self.render_inputs(strategy);
    self.render_profile(state, strategy);
    self.render_entry_filter(strategy);

    const auto valid = state.get_model_if_present(strategy.model_handle()) &&
                       state.get_profile_if_present(strategy.profile_handle());
    ImGui::BeginDisabled(!valid);
    if(ImGui::Button(self.editing_handle_ ? "OK" : "Create Strategy"))
      self.submit(context, true);
    ImGui::EndDisabled();
    ImGui::SameLine();
    if(ImGui::Button("Cancel")) self.reset();
    if(self.editing_handle_) {
      const auto& original = state.get_strategy(*self.editing_handle_);
      const auto has_draft = self.has_changes(strategy, original);
      ImGui::SameLine();
      const auto action = ui::apply_reset_button(has_draft, valid);
      if(action == ui::DraftAction::Apply) self.submit(context, false);
      else if(action == ui::DraftAction::Reset)
        self.editing_ = std::make_shared<backtest::Strategy>(original);
    }
    if(!valid) ui::validation_message("Select both a model and a profile before saving.");
  }

  void render_name(this StrategiesWindow&, backtest::Strategy& strategy)
  {
    ui::form_section("Strategy Details", "A Strategy combines a Model, its input values, a Profile, and an Entry Filter.");
    auto name = strategy.name();
    ui::field_label("Name");
    ImGui::InputTextWithHint("##strategy_name", "Unnamed", &name);
    strategy.name(std::move(name));
  }

  void render_model(this StrategiesWindow&, const ApplicationState& state,
                    backtest::Strategy& strategy)
  {
    ui::form_section("Model", "Changing the Model resets this Strategy's inputs to that Model's defaults.");
    const auto* selected = state.get_model_if_present(strategy.model_handle());
    ui::field_label("Model");
    if(ImGui::BeginCombo("##strategy_model", selected ? selected->name().c_str() : "Select a model")) {
      for(const auto handle : state.get_model_handles()) {
        const auto& model = state.get_model(handle);
        if(ImGui::Selectable(model.name().c_str(), handle == strategy.model_handle()) &&
           handle != strategy.model_handle())
          backtest::assign_strategy_model(strategy, handle, model);
      }
      ImGui::EndCombo();
    }
  }

  void render_inputs(this StrategiesWindow&, backtest::Strategy& strategy)
  {
    ui::form_section("Model Inputs", "These values override the selected Model's input defaults for this Strategy.");
    auto inputs = strategy.inputs();
    if(inputs.empty()) {
      ImGui::TextDisabled("The selected model has no inputs.");
      return;
    }
    for(auto index = std::size_t{}; index < inputs.size(); ++index) {
      auto& input = inputs[index];
      ImGui::PushID(static_cast<int>(index));
      ui::field_label(input.label().c_str());
      if(input.representation() == pludux::NumericInputNode::ValueRepresentation::Decimal) {
        auto value = input.value();
        if(ImGui::InputDouble("##value", &value)) input.value(value);
      } else if(input.representation() == pludux::NumericInputNode::ValueRepresentation::SignedInteger) {
        auto value = static_cast<std::int64_t>(input.value());
        if(ImGui::InputScalar("##value", ImGuiDataType_S64, &value)) input.value(static_cast<double>(value));
      } else {
        auto value = static_cast<std::uint64_t>(input.value());
        if(ImGui::InputScalar("##value", ImGuiDataType_U64, &value)) input.value(static_cast<double>(value));
      }
      ImGui::PopID();
    }
    strategy.inputs(std::move(inputs));
  }

  void render_profile(this StrategiesWindow&, const ApplicationState& state,
                      backtest::Strategy& strategy)
  {
    ui::form_section("Profile", "The Profile controls position sizing and capital protection.");
    const auto* selected = state.get_profile_if_present(strategy.profile_handle());
    ui::field_label("Profile");
    if(ImGui::BeginCombo("##strategy_profile", selected ? selected->name().c_str() : "Select a profile")) {
      for(const auto handle : state.get_profile_handles()) {
        const auto& profile = state.get_profile(handle);
        if(ImGui::Selectable(profile.name().c_str(), handle == strategy.profile_handle()))
          strategy.profile_handle(handle);
      }
      ImGui::EndCombo();
    }
  }

  void render_entry_filter(this StrategiesWindow&, backtest::Strategy& strategy)
  {
    ui::form_section("Entry Filter", "Decide whether a fresh initial Requested Order may enter Portfolio ranking.");
    auto entry_filter = strategy.entry_filter();
    if(backtest::gui::entry_filter_node_editor(entry_filter))
      strategy.entry_filter(std::move(entry_filter));
  }

  auto dependent_system_count(this StrategiesWindow&, const ApplicationState& state,
                              backtest::StrategyStoreHandle handle) -> std::size_t
  {
    auto count = std::size_t{};
    for(const auto system_handle : state.get_system_handles()) {
      const auto& system = state.get_system(system_handle);
      for(auto index = std::size_t{}; index < system.strategy_count(); ++index) {
        if(system.strategy_handle(index) == handle) {
          ++count;
          break;
        }
      }
    }
    return count;
  }

  auto has_changes(this StrategiesWindow&, const backtest::Strategy& current,
                   const backtest::Strategy& original) -> bool
  {
    if(current.name() != original.name() ||
       current.model_handle() != original.model_handle() ||
       current.profile_handle() != original.profile_handle() ||
       current.entry_filter() != original.entry_filter() ||
       current.inputs().size() != original.inputs().size()) {
      return true;
    }
    for(auto index = std::size_t{}; index < current.inputs().size(); ++index) {
      const auto& input = current.inputs()[index];
      const auto& original_input = original.inputs()[index];
      if(input.label() != original_input.label() ||
         input.representation() != original_input.representation() ||
         input.value() != original_input.value()) {
        return true;
      }
    }
    return false;
  }

  void submit(this StrategiesWindow& self, WindowContext& context, bool reset_on_success)
  {
    auto value = *self.editing_;
    if(!context.app_state().get_model_if_present(value.model_handle()) ||
       !context.app_state().get_profile_if_present(value.profile_handle())) return;
    if(value.name().empty()) value.name("Unnamed");
    context.push_edit(self.editing_handle_ ? "Edit Strategy" : "Add Strategy",
                      [handle = self.editing_handle_, value = std::move(value)](ApplicationState& candidate) mutable {
      if(handle) candidate.update_strategy(*handle, std::move(value));
      else candidate.add_strategy(std::move(value));
    });
    if(reset_on_success) self.reset();
  }

  void reset(this StrategiesWindow& self) noexcept
  {
    self.editing_handle_.reset();
    self.editing_.reset();
  }
};

} // namespace pludux::apps

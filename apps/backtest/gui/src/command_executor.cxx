module;

#include <algorithm>
#include <cstddef>
#include <functional>
#include <list>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

export module pludux.apps.backtest.command_executor;

import pludux.apps.backtest.application_state;
import pludux.apps.backtest.state_diff;

export namespace pludux::apps {

using ApplicationAction = std::function<void(ApplicationState&)>;
using ApplicationStateLoader = std::function<ApplicationState()>;

struct EditCommand {
  std::string label;
  ApplicationAction action;
  std::optional<std::string> merge_key;
};

struct ViewCommand {
  ApplicationAction action;
};

struct ReplaceApplicationCommand {
  ApplicationStateLoader loader;
};

struct UndoCommand {};
struct RedoCommand {};

using CommandVariant = std::variant<EditCommand,
                                    ViewCommand,
                                    ReplaceApplicationCommand,
                                    UndoCommand,
                                    RedoCommand>;

enum class ExecutionEffect {
  None,
  ViewChanged,
  DocumentChanged,
  ApplicationReplaced
};

class CommandExecutor {
public:
  static constexpr auto history_limit = std::size_t{100};

  auto execute(this CommandExecutor& self, ApplicationState& app_state)
   -> ExecutionEffect
  {
    auto effect = ExecutionEffect::None;
    while(!self.pending_commands_.empty()) {
      auto command = std::move(self.pending_commands_.front());
      self.pending_commands_.pop_front();
      const auto command_effect = std::visit(
       [&]<typename TCommand>(TCommand&& value) {
         return self.visit_command(std::forward<TCommand>(value), app_state);
       },
       std::move(command));
      effect = std::max(effect, command_effect);
    }
    return effect;
  }

  void push(this CommandExecutor& self, CommandVariant command)
  {
    self.pending_commands_.push_back(std::move(command));
  }

  auto can_undo(this const CommandExecutor& self) noexcept -> bool
  {
    return !self.undo_stack_.empty();
  }

  auto can_redo(this const CommandExecutor& self) noexcept -> bool
  {
    return !self.redo_stack_.empty();
  }

  auto undo_label(this const CommandExecutor& self) noexcept
   -> const std::string*
  {
    return self.undo_stack_.empty() ? nullptr : &self.undo_stack_.back().label;
  }

  auto redo_label(this const CommandExecutor& self) noexcept
   -> const std::string*
  {
    return self.redo_stack_.empty() ? nullptr : &self.redo_stack_.back().label;
  }

  auto undo_size(this const CommandExecutor& self) noexcept -> std::size_t
  {
    return self.undo_stack_.size();
  }

  auto redo_size(this const CommandExecutor& self) noexcept -> std::size_t
  {
    return self.redo_stack_.size();
  }

private:
  struct HistoryEntry {
    std::string label;
    StateDiff diff;
    std::optional<std::string> merge_key;
  };

  std::list<CommandVariant> pending_commands_;
  std::vector<HistoryEntry> undo_stack_;
  std::vector<HistoryEntry> redo_stack_;
  bool can_coalesce_{};

  static void push_bounded(std::vector<HistoryEntry>& stack, HistoryEntry entry)
  {
    if(stack.size() == history_limit) {
      stack.erase(stack.begin());
    }
    stack.push_back(std::move(entry));
  }

  void end_coalescing(this CommandExecutor& self) noexcept
  {
    self.can_coalesce_ = false;
  }

  auto visit_command(this CommandExecutor& self,
                     EditCommand command,
                     ApplicationState& app_state) -> ExecutionEffect
  {
    auto candidate = app_state;
    command.action(candidate);
    auto inverse = create_state_diff(candidate, app_state);
    const auto document_changed = !inverse.empty(candidate);
    const auto view_changed = candidate.view_state() != app_state.view_state();
    app_state = std::move(candidate);

    if(!document_changed) {
      self.end_coalescing();
      return view_changed ? ExecutionEffect::ViewChanged
                          : ExecutionEffect::None;
    }

    const auto coalesce =
     self.can_coalesce_ && command.merge_key && !self.undo_stack_.empty() &&
     self.undo_stack_.back().merge_key == command.merge_key;
    if(!coalesce) {
      push_bounded(self.undo_stack_,
                   HistoryEntry{std::move(command.label),
                                std::move(inverse),
                                std::move(command.merge_key)});
    }
    self.redo_stack_.clear();
    self.can_coalesce_ = self.undo_stack_.back().merge_key.has_value();
    return ExecutionEffect::DocumentChanged;
  }

  auto visit_command(this CommandExecutor& self,
                     ViewCommand command,
                     ApplicationState& app_state) -> ExecutionEffect
  {
    const auto previous_view = app_state.view_state();
    command.action(app_state);
    self.end_coalescing();
    return previous_view == app_state.view_state()
            ? ExecutionEffect::None
            : ExecutionEffect::ViewChanged;
  }

  auto visit_command(this CommandExecutor& self,
                     ReplaceApplicationCommand command,
                     ApplicationState& app_state) -> ExecutionEffect
  {
    auto replacement = command.loader();
    app_state = std::move(replacement);
    self.undo_stack_.clear();
    self.redo_stack_.clear();
    self.end_coalescing();
    return ExecutionEffect::ApplicationReplaced;
  }

  auto visit_command(this CommandExecutor& self,
                     UndoCommand,
                     ApplicationState& app_state) -> ExecutionEffect
  {
    self.end_coalescing();
    if(self.undo_stack_.empty()) {
      return ExecutionEffect::None;
    }

    auto entry = std::move(self.undo_stack_.back());
    self.undo_stack_.pop_back();
    const auto previous = app_state;
    app_state = entry.diff.apply(previous);
    auto inverse = create_state_diff(app_state, previous);
    push_bounded(
     self.redo_stack_,
     HistoryEntry{std::move(entry.label), std::move(inverse), std::nullopt});
    return ExecutionEffect::DocumentChanged;
  }

  auto visit_command(this CommandExecutor& self,
                     RedoCommand,
                     ApplicationState& app_state) -> ExecutionEffect
  {
    self.end_coalescing();
    if(self.redo_stack_.empty()) {
      return ExecutionEffect::None;
    }

    auto entry = std::move(self.redo_stack_.back());
    self.redo_stack_.pop_back();
    const auto previous = app_state;
    app_state = entry.diff.apply(previous);
    auto inverse = create_state_diff(app_state, previous);
    push_bounded(
     self.undo_stack_,
     HistoryEntry{std::move(entry.label), std::move(inverse), std::nullopt});
    return ExecutionEffect::DocumentChanged;
  }
};

} // namespace pludux::apps

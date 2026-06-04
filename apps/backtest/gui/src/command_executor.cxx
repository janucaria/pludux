module;

#include <functional>
#include <list>
#include <string>
#include <utility>
#include <variant>
#include <vector>

// NOTE: emscripten have a bug that causes compilation got crashed when
// std::queue or std::stack is used in the code. #include <queue> #include
// <stack>

export module pludux.apps.backtest:command_executor;

import :application_state;
import :state_diff;

export namespace pludux::apps {

using ActionCommand = std::function<void(ApplicationState&)>;

struct UndoCommand {};

struct RedoCommand {};

using CommandVariant = std::variant<ActionCommand, UndoCommand, RedoCommand>;

class CommandExecutor {
public:
  auto execute(this CommandExecutor& self, ApplicationState& app_state) -> bool
  {
    if(self.pending_commands_.empty()) {
      return false;
    }

    while(!self.pending_commands_.empty()) {
      auto command_variant = std::move(self.pending_commands_.front());
      self.pending_commands_.pop_front();

      std::visit(
       [&]<typename TCommand>(TCommand&& command) {
         self.visit_command(std::forward<TCommand>(command), app_state);
       },
       command_variant);
    }
    return true;
  }

  void push(this CommandExecutor& self, CommandVariant command_variant)
  {
    self.pending_commands_.push_back(std::move(command_variant));
  }

  auto can_undo(this const CommandExecutor& self) -> bool
  {
    return !self.undo_stack_.empty();
  }

  auto can_redo(this const CommandExecutor& self) -> bool
  {
    return !self.redo_stack_.empty();
  }

private:
  std::list<CommandVariant> pending_commands_;
  std::vector<StateDiff> undo_stack_;
  std::vector<StateDiff> redo_stack_;

  void visit_command(this CommandExecutor& self,
                     ActionCommand command,
                     ApplicationState& app_state)
  {
    auto prev_state = app_state;
    command(app_state);
    const auto state_diff = create_state_diff(app_state, prev_state);

    self.undo_stack_.push_back(state_diff);
    self.redo_stack_.clear();
  }

  void visit_command(this CommandExecutor& self,
                     UndoCommand,
                     ApplicationState& app_state)
  {
    if(self.undo_stack_.empty()) {
      return;
    }

    const auto prev_state = app_state;
    const auto& state_diff = self.undo_stack_.back();
    app_state = state_diff.apply(prev_state);
    self.undo_stack_.pop_back();

    const auto new_state_diff = create_state_diff(app_state, prev_state);
    self.redo_stack_.push_back(new_state_diff);
  }

  void visit_command(this CommandExecutor& self,
                     RedoCommand,
                     ApplicationState& app_state)
  {
    if(self.redo_stack_.empty()) {
      return;
    }

    const auto prev_state = app_state;
    const auto& state_diff = self.redo_stack_.back();
    app_state = state_diff.apply(prev_state);
    self.redo_stack_.pop_back();

    const auto new_state_diff = create_state_diff(app_state, prev_state);
    self.undo_stack_.push_back(new_state_diff);
  }
};

} // namespace pludux::apps
